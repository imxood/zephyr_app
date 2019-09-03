#include <zephyr.h>
#include <device.h>
#include <init.h>
#include <logging/log.h>
#include <net/net_pkt.h>
#include <net/net_if.h>
#include <net/wifi.h>
#include <net/wifi_mgmt.h>
#include <net/buf.h>
#include <net/net_offload.h>
#include <drivers/modem/modem_receiver.h>

#define MAX_DATA_SIZE 1600
#define BUF_ALLOC_TIMEOUT K_SECONDS(1)
#define ESP32_OFFLOAD_MAX_SOCKETS 4

LOG_MODULE_REGISTER(esp32, LOG_LEVEL_DBG);

K_THREAD_STACK_DEFINE(esp32_work_q_stack, 1024);

#define CMD_HANDLER_NO_SKIP(cmd_, cb_) { \
	.cmd = cmd_, \
	.cmd_len = (u16_t)sizeof(cmd_)-1, \
	.func = on_cmd_ ## cb_, \
	.skip = 0 \
}

#define CMD_HANDLER(cmd_, cb_) { \
	.cmd = cmd_, \
	.cmd_len = (u16_t)sizeof(cmd_)-1, \
	.func = on_cmd_ ## cb_, \
	.skip = 1 \
}

/* net bufs */
// NET_BUF_POOL_DEFINE(esp32_recv_pool, 12, 128, 0, NULL);

static u8_t mdm_recv_buf[MAX_DATA_SIZE];

K_THREAD_STACK_DEFINE(rx_thread_stack, 512);
struct k_thread rx_thread;

/****************************** data struct define start ******************************/

enum esp32_request {
	ESP32_REQ_SCAN,
	ESP32_REQ_CONNECT,
	ESP32_REQ_DISCONNECT,
	ESP32_REQ_NONE
};

enum esp32_security_type {
	ESP32_SEC_OPEN,
	ESP32_SEC_WEP,
	ESP32_SEC_WPA,
	ESP32_SEC_WPA2_AES,
	ESP32_SEC_WPA2_MIXED,
	ESP32_SEC_MAX
};

struct esp32_sta {
	char ssid[WIFI_SSID_MAX_LEN + 1];
	enum esp32_security_type security;
	char pass[65];
	bool connected;
	uint8_t channel;
};

struct sockets {
	uint8_t index;
	struct net_context *context;
	sa_family_t family;
	enum net_sock_type type;
	enum net_ip_protocol ip_proto;
	uint8_t connected;
};

struct esp32_dev {
	struct net_if* iface;
	struct in_addr ip;
	struct in_addr gw;
	struct in_addr netmask;
	u8_t mac[6];

	struct k_work_q work_q;
	struct k_work request_work;

	/* modem stuff */
	struct mdm_receiver_context mdm_ctx;

	char buf[MAX_DATA_SIZE];

	enum esp32_request req;
	struct esp32_sta sta;

	scan_result_cb_t *scan_cb;

	/* dev atomic operation */
	struct k_mutex mutex;

	/* wait for response */
	struct k_sem response_sem;

	// struct net_buf *buf;

	int last_error;

	struct sockets sockets[ESP32_OFFLOAD_MAX_SOCKETS];

	int sending_data;
};

struct cmd_handler {
	const char *cmd;
	u16_t cmd_len;
	void (*func)(struct net_buf **buf, u16_t len);
	int skip;
};

/****************************** data struct define end ******************************/



/****************************** global data start ******************************/
static struct esp32_dev esp32_dev;
/****************************** global data end ******************************/




/* *************************** Echo Handler for commands without related sockets ******************************** */

static void on_cmd_atcmdecho_nosock(struct net_buf **buf, u16_t len)
{
}

static void on_cmd_esp32_ready(struct net_buf **buf, u16_t len)
{
	k_sem_give(&esp32_dev.response_sem);
}

static void on_cmd_ip_addr_get(struct net_buf **buf, u16_t len)
{
		k_delayed_work_submit_to_queue(&esp32_dev.work_q, &esp32_dev.request_work, K_SECONDS(2));
}

static void on_cmd_conn_open(struct net_buf **buf, u16_t len)
{
	int id;
	u8_t msg[2];

	net_buf_linearize(msg, 1, *buf, 0, 1);
	msg[1] = '\0';
	id = atoi(msg);

	esp32_dev.sockets[id].connected = 1;
}

static void on_cmd_conn_closed(struct net_buf **buf, u16_t len)
{
	int id;
	u8_t msg[2];
	struct sockets *sock = NULL;

	net_buf_linearize(msg, 1, *buf, 0, 1);
	msg[1] = '\0';
	id = atoi(msg);

	sock = &esp32_dev.sockets[id];
	sock->connected = 0;
}

static void on_cmd_sent_bytes(struct net_buf **buf, u16_t len)
{
	char temp[32];
	char *tok;

	net_buf_linearize(temp, 32, *buf, 0, len);
	temp[len] = '\0';

	tok = strTok(temp, " ");
	while(tok) {
		tok = strTok(NULL, " ");
	}
}

static void on_cmd_wifi_scan_resp(struct net_buf **buf, u16_t len)
{
	int i;
	char temp[32];
	struct wifi_scan_result result;
	int delimiters[6];
	int slen;

	/* ecn, ssid, rssi, mac, channel, freq */

	delimiters[0] = 1;
	for (i = 1; i < 6; i++) {
		delimiters[i] = net_buf_find_next_delimiter(*buf, ',',
					delimiters[i-1] + 1);
		if (delimiters[i] == -1) {
			return;
		}
		delimiters[i]++;
	}

	/* ecn */
	net_buf_linearize(temp, 1, *buf, delimiters[0],
			  delimiters[1] - delimiters[0]);
	if (temp[0] != '0') {
		result.security = WIFI_SECURITY_TYPE_PSK;
	} else {
		result.security = WIFI_SECURITY_TYPE_PSK;
	}

	/* ssid */
	slen = delimiters[2] - delimiters[1] - 3;
	net_buf_linearize(result.ssid, 32, *buf, delimiters[1] + 1,
			  slen);
	result.ssid_length = slen;

	/* rssi */
	slen = delimiters[3] - delimiters[2];
	net_buf_linearize(temp, 32, *buf, delimiters[2], slen);
	temp[slen] = '\0';
	result.rssi = strtol(temp, NULL, 10);

	/* channel */
	slen = delimiters[5] - delimiters[4];
	net_buf_linearize(temp, 32, *buf, delimiters[4], slen);
	temp[slen] = '\0';
	result.channel = strtol(temp, NULL, 10);

	/* issue callback to report scan results */
	if (esp32_dev.scan_cb) {
		esp32_dev.scan_cb(esp32_dev.iface, 0, &result);
	}
}

static const char nm_label[] = "netmask";
static const char gw_label[] = "gateway";
static const char ip_label[] = "ip";

static void on_cmd_ip_addr_resp(struct net_buf **buf, u16_t len)
{
	char ip_addr[] = "xxx.xxx.xxx.xxx";
	int d[3];
	int slen;

	d[0] = net_buf_find_next_delimiter(*buf, ':', 0);
	d[1] = net_buf_find_next_delimiter(*buf, '\"', d[0] + 1);
	d[2] = net_buf_find_next_delimiter(*buf, '\"', d[1] + 1);

	slen = d[2] - d[1] - 1;

	net_buf_linearize(ip_addr, 16, *buf, d[0] + 2, slen);
	ip_addr[slen] = '\0';

	if (net_buf_ncmp(*buf, nm_label, strlen(nm_label)) == 0) {
		inet_pton(AF_INET, ip_addr,
			  &esp32_dev.netmask);
	} else if (net_buf_ncmp(*buf, ip_label, strlen(ip_label)) == 0) {
		inet_pton(AF_INET, ip_addr,
			  &esp32_dev.ip);
	} else if (net_buf_ncmp(*buf, gw_label, strlen(gw_label)) == 0) {
		inet_pton(AF_INET, ip_addr,
			  &esp32_dev.gw);
	} else {
		return;
	}
}

static void on_cmd_mac_addr_resp(struct net_buf **buf, u16_t len)
{
	int i;
	char octet[] = "xx";
	for (i = 0; i < 6; i++) {
		net_buf_pull_u8(*buf);
		octet[0]= net_buf_pull_u8(*buf);
		octet[1]= net_buf_pull_u8(*buf);
		esp32_dev.mac[i] = strtoul(octet, NULL, 16);
	}
}

static void on_cmd_sendok(struct net_buf **buf, u16_t len)
{
	esp32_dev.last_error = 0;

	/* only signal semaphore if we are not waiting on a data send command */
	k_sem_give(&esp32_dev.response_sem);
}

/* Handler: OK */
static void on_cmd_sockok(struct net_buf **buf, u16_t len)
{
	esp32_dev.last_error = 0;

	if (!esp32_dev.sending_data) {
		k_sem_give(&esp32_dev.response_sem);
	}
}

/* Handler: ERROR */
static void on_cmd_sockerror(struct net_buf **buf, u16_t len)
{
	esp32_dev.last_error = -EIO;
	k_sem_give(&esp32_dev.response_sem);
}

static void on_cmd_wifi_connected_resp(struct net_buf **buf, u16_t len)
{
	wifi_mgmt_raise_connect_result_event(esp32_dev.iface, 0);
}

static void on_cmd_wifi_disconnected_resp(struct net_buf **buf, u16_t len)
{
	wifi_mgmt_raise_disconnect_result_event(esp32_dev.iface, 0);
}

/* ******************************************************************** */




/****************************** esp32 function define start ******************************/

static inline void esp32_lock(const struct esp32_dev *esp32)
{
	k_mutex_lock(&esp32->mutex, K_FOREVER);
}

static inline void esp32_unlock(const struct esp32_dev *esp32)
{
	k_mutex_unlock(&esp32->mutex);
}

static inline struct net_buf *read_rx_allocator(s32_t timeout, void *user_data)
{
	return net_buf_alloc((struct net_buf_pool *)user_data, timeout);
}

/* Send an AT command with a series of response handlers */
static int send_at_cmd(struct esp32_dev *esp32, const u8_t *data, int timeout)
{
	int ret;

	mdm_receiver_send(&esp32->mdm_ctx, data, strlen(data));
	mdm_receiver_send(&esp32->mdm_ctx, "\r\n", 2);

	if (timeout == K_NO_WAIT) {
		return 0;
	}

	ret = k_sem_take(&esp32->response_sem, timeout);

	if (ret == 0) {
		ret = esp32->last_error;
		LOG_DBG("waitting for response timeout");
	} else if (ret == -EAGAIN) {
		ret = -ETIMEDOUT;
	}

	return ret;
}

static void esp32_read_rx(struct esp32_dev *esp32)
{
	size_t bytes_read = 0;
	int ret;

	esp32->buf[0] = '\0';

	/* read all of the data from mdm_receiver */
	while (true) {
		ret = mdm_receiver_recv(&esp32->mdm_ctx,
					esp32->buf,
					sizeof(esp32->buf),
					&bytes_read);
		if (ret) {
			LOG_ERR("ERROR: %d", ret);
			/* mdm_receiver buffer is empty */
			break;
		}
		if (bytes_read == 0) {
			break;
		}
	}
}

static int esp32_reset(struct esp32_dev *esp32)
{
	LOG_DBG("esp32_reset");
	esp32_lock(esp32);

	u8_t *cmd = "AT+RST";
	LOG_DBG("run esp32 command: %s", cmd);

	send_at_cmd(esp32, cmd, 3000);

	LOG_DBG("run esp32 command: successful!");

	esp32_unlock(esp32);

	LOG_DBG("esp32_reset leave");

	return 0;
}

static int esp32_connect(struct esp32_dev *esp32)
{
	LOG_DBG("Connecting to %s (pass=%s)", esp32->sta.ssid, esp32->sta.pass);

	esp32_lock(esp32);


	LOG_DBG("Connected!");

	esp32_unlock(esp32);
	return 0;

error:
	esp32_unlock(esp32);
	return -EIO;
}


/* RX thread */
static void esp32_rx(void *p1, void *p2, void *p3)
{

	struct net_buf *rx_buf = NULL;
	struct net_buf *frag = NULL;
	int i;
	u16_t offset, len;

	static const struct cmd_handler handlers[] = {
		CMD_HANDLER("AT+RST", atcmdecho_nosock),
		CMD_HANDLER("ATE1", atcmdecho_nosock),
		CMD_HANDLER("OK", sockok),
		CMD_HANDLER("ERROR", sockerror),
		CMD_HANDLER("FAIL", sockerror),
		CMD_HANDLER("SEND FAIL", sockerror),
		CMD_HANDLER("WIFI GOT IP", ip_addr_get),
		CMD_HANDLER("AT+CWJAP_CUR=", atcmdecho_nosock),
		CMD_HANDLER("WIFI CONNECTED", wifi_connected_resp),
		CMD_HANDLER("WIFI DISCONNECT",wifi_disconnected_resp),
		CMD_HANDLER("SEND OK", sendok),
		CMD_HANDLER("link is not valid", atcmdecho_nosock),
		CMD_HANDLER("busy p...", atcmdecho_nosock),
		CMD_HANDLER("busy s...", atcmdecho_nosock),
		CMD_HANDLER("ready", esp32_ready),
		CMD_HANDLER("AT+CIPAPMAC_CUR?", atcmdecho_nosock),
		CMD_HANDLER("+CIPAPMAC_CUR:", mac_addr_resp),
		CMD_HANDLER("AT+CIPSTA_CUR?", atcmdecho_nosock),
		CMD_HANDLER("+CIPSTA_CUR:", ip_addr_resp),
		CMD_HANDLER("AT+CWLAP", atcmdecho_nosock),
		CMD_HANDLER("+CWLAP:", wifi_scan_resp),
		CMD_HANDLER("+CWLAP:", wifi_scan_resp),
		CMD_HANDLER("AT+CIPMUX", atcmdecho_nosock),
		CMD_HANDLER("AT+CWQAP", atcmdecho_nosock),
		CMD_HANDLER("AT+CIPSEND=", atcmdecho_nosock),
		CMD_HANDLER_NO_SKIP("0,CONNECT", conn_open),
		CMD_HANDLER_NO_SKIP("1,CONNECT", conn_open),
		CMD_HANDLER_NO_SKIP("2,CONNECT", conn_open),
		CMD_HANDLER_NO_SKIP("3,CONNECT", conn_open),
		CMD_HANDLER_NO_SKIP("4,CONNECT", conn_open),
		CMD_HANDLER_NO_SKIP("0,CLOSED", conn_closed),
		CMD_HANDLER_NO_SKIP("1,CLOSED", conn_closed),
		CMD_HANDLER_NO_SKIP("2,CLOSED", conn_closed),
		CMD_HANDLER_NO_SKIP("3,CLOSED", conn_closed),
		CMD_HANDLER_NO_SKIP("4,CLOSED", conn_closed),
		CMD_HANDLER("Recv ", sent_bytes),
	};


	LOG_DBG("run RX thread");

	struct esp32_dev *esp32 = (struct esp32_dev *)p1;

	struct net_buf *rx_buf = NULL;

	while (true) {

		k_sem_take(&esp32->mdm_ctx.rx_sem, K_FOREVER);

		esp32_read_rx(esp32);

		if(strlen(esp32->buf) == 0) {
			printk("esp32->buf: NULL\n");
		}else{
			printk("esp32->buf: %s", esp32->buf);
		}

		k_sem_give(&esp32->response_sem);

		/* give up time if we have a solid stream of data */
		k_yield();
	}
}

/****************************** esp32 function define end ******************************/


/****************************** net offload interface define start ******************************/

int esp32_get(sa_family_t family, enum net_sock_type type, enum net_ip_protocol ip_proto, struct net_context **context)
{
	struct sockets *socket = NULL;

	if(family != AF_INET) {
		LOG_ERR("Only AF_INET is supperted!");
		return -EPFNOSUPPORT;
	}
	if(ip_proto != IPPROTO_TCP) {
		LOG_ERR("Only IPPROTO_TCP is supperted!");
		return -EPROTONOSUPPORT;
	}

	esp32_lock(&esp32_dev);

	for(int i = 0; i < ESP32_OFFLOAD_MAX_SOCKETS; i++) {
		if(!esp32_dev.sockets[i].context) {
			socket = esp32_dev.sockets + i;
			socket->index = i;
			socket->context = *context;
			socket->connected = 0;
			socket->family = family;
			socket->type = type;
			socket->ip_proto = ip_proto;
			break;
		}
	}

	if(!socket) {
		LOG_DBG("socket is NULL, no mem.");
		esp32_unlock(&esp32_dev);
		return -ENOMEM;
	}

	esp32_unlock(&esp32_dev);
	return 0;
}


static int esp32_sendto(struct net_pkt *pkt, const struct sockaddr *dst_addr,
			socklen_t addrlen, net_context_send_cb_t cb,
			s32_t timeout,
			// void *token,
			void *user_data)
{
	u8_t send_msg[48];
	struct net_context *context = net_pkt_context(pkt);
	struct socket_data *data;
	struct net_buf *frag;
	int len;
	int ret = -EFAULT;
	int send_len;
	u8_t addr_str[32];
	int id;

	if (!context || !context->offload_context) {
		return -EINVAL;
	}

	data = context->offload_context;
	id = data - ictx.socket_data;

	if (!data->connected) {
		esp32_connect(data->context,
			dst_addr,
			addrlen,
			NULL,
			0, NULL);
	}

	frag = pkt->frags;

	send_len = net_buf_frags_len(frag);
	ictx.sending_data = 1;
	if (net_context_get_type(context) == SOCK_STREAM) {
		len = sprintf(send_msg, "AT+CIPSEND=%d,%d", id,
			      net_buf_frags_len(frag));
	} else {
		inet_ntop(data->dst.sa_family, &net_sin(&data->dst)->sin_addr,
			     addr_str, sizeof(addr_str));
		len = sprintf(send_msg, "AT+CIPSEND=%d,%d,\"%s\",%d",
			id, net_buf_frags_len(frag), addr_str,
				ntohs(net_sin(&data->dst)->sin_port));
	}

	ret = send_at_cmd(NULL, send_msg, MDM_CMD_TIMEOUT * 2);
	if (ret < 0) {
		ret = -EINVAL;
		goto ret_early;
	}

	ictx.sending_data = 0;
	k_sem_reset(&ictx.response_sem);
	k_mutex_lock(&dev_mutex, K_FOREVER);
	while (send_len && frag) {
		if (frag->len > send_len) {
			mdm_receiver_send(&ictx.mdm_ctx, frag->data,
					  send_len);

			send_len = 0;
			break;
		} else {
			mdm_receiver_send(&ictx.mdm_ctx, frag->data,
					  frag->len);
			send_len -= frag->len;
			frag = frag->frags;
		}
	}
	k_mutex_unlock(&dev_mutex);

	ret = k_sem_take(&ictx.response_sem, MDM_CMD_TIMEOUT*2);

	if (ret == 0) {
		ret = ictx.last_error;
	} else if (ret == -EAGAIN) {
		ret = -ETIMEDOUT;
	}

ret_early:
	net_pkt_unref(pkt);
	if (cb) {
		// typedef void (*net_context_send_cb_t)(struct net_context *context,
		// 		      int status,
		// 		      void *user_data);
		cb(context, ret, user_data);
		// cb(context, ret, token, user_data);
	}

	return ret;
}

static struct net_offload esp32_offload = {
	.get		= esp32_get,
	// .bind		= esp32_bind,
	// .listen		= esp32_listen,
	// .connect	= esp32_connect,
	// .accept		= esp32_accept,
	// .send		= esp32_send,
	.sendto		= esp32_sendto,
	// .recv		= esp32_recv,
	// .put		= esp32_put,
};


/****************************** net offload interface define end ******************************/



/****************************** interface define start ******************************/

static int esp32_mgmt_scan(struct device *dev, scan_result_cb_t *cb)
{
	struct esp32_dev *esp32 = dev->driver_data;
	esp32->scan_cb = cb;
	esp32->req = ESP32_REQ_SCAN;
	return 0;
}

static int esp32_mgmt_connect(struct device *dev, struct wifi_connect_req_params *params)
{
	return 0;
}

static int esp32_mgmt_disconnect(struct device *dev)
{
	return 0;
}

static int esp32_mgmt_ap_enable(struct device *dev, struct wifi_connect_req_params *params)
{
	return 0;
}

static int esp32_mgmt_ap_disable(struct device *dev)
{
	return 0;
}

static void esp32_iface_init(struct net_if *iface)
{
	LOG_DBG("esp32_iface_init");
	esp32_lock(&esp32_dev);
	esp32_dev.iface = iface;

	esp32_dev.iface->if_dev->offload = &esp32_offload;

	esp32_unlock(&esp32_dev);
}

/****************************** interface define end ******************************/



static void esp32_request_work(struct k_work *item)
{
	struct esp32_dev *esp32;
	int err = 0;

	esp32 = CONTAINER_OF(item, struct esp32_dev, work_q);

	switch (esp32->req) {
	case ESP32_REQ_SCAN:
		err = esp32_connect(esp32);
		break;

	case ESP32_REQ_CONNECT:
		break;

	case ESP32_REQ_DISCONNECT:
		break;

	case ESP32_REQ_NONE:
	default:
		break;
	}
}



/******************************  net device offload init start ******************************/

static const struct net_wifi_mgmt_offload esp32_offload_api = {
	.iface_api.init = esp32_iface_init,
	.scan		= esp32_mgmt_scan,
	.connect	= esp32_mgmt_connect,
	.disconnect	= esp32_mgmt_disconnect,
	.ap_enable	= esp32_mgmt_ap_enable,
	.ap_disable	= esp32_mgmt_ap_disable,
};

static int esp32_init(struct device *dev)
{
	struct esp32_dev *esp32 = dev->driver_data;

	LOG_DBG("esp32_init");

	/* init */

	k_sem_init(&esp32->response_sem, 0, 1);
	k_mutex_init(&esp32->mutex);

	k_work_q_start(&esp32->work_q, esp32_work_q_stack, K_THREAD_STACK_SIZEOF(esp32_work_q_stack), CONFIG_SYSTEM_WORKQUEUE_PRIORITY - 1);
	k_work_init(&esp32->request_work, esp32_request_work);


	if (mdm_receiver_register(&esp32->mdm_ctx, CONFIG_WIFI_ESP32_UART_DEVICE, mdm_recv_buf, sizeof(mdm_recv_buf)) < 0) {
		LOG_ERR("Error registering modem receiver");
		return -EINVAL;
	} else {
		LOG_DBG("registering modem receiver success");
	}

	// /* init rx thread */
	k_thread_create(&rx_thread, rx_thread_stack, K_THREAD_STACK_SIZEOF(rx_thread_stack), esp32_rx, esp32, NULL, NULL, CONFIG_SYSTEM_WORKQUEUE_PRIORITY - 1, 0, 0);

	k_sleep(100);

	esp32_reset(esp32);

	return 0;
}

NET_DEVICE_OFFLOAD_INIT(esp32, "esp32", esp32_init, &esp32_dev, NULL, 0, &esp32_offload_api, 1600);

/******************************  net device offload init end ******************************/
