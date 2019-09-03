#pragma once

#include <zephyr.h>
#include <kernel.h>
#include <net/wifi.h>
#include <net/wifi_mgmt.h>
#include <net/net_context.h>
#include <net/net_pkt.h>
#include <net/net_offload.h>
#include <net/socks.h>
#include <net/udp.h>

#define MAX_DATA_SIZE 1600

#define WIFI_OFFLOAD_MAX_SOCKETS 4
/*
enum wifi_security_type {
	WIFI_SEC_OPEN,
	WIFI_SEC_WEP,
	WIFI_SEC_WPA,
	WIFI_SEC_WPA2_AES,
	WIFI_SEC_WPA2_MIXED,
	WIFI_SEC_MAX
};

enum wifi_request {
	WIFI_REQ_SCAN,
	WIFI_REQ_CONNECT,
	WIFI_REQ_DISCONNECT,
	WIFI_REQ_NONE
};

enum wifi_role {
	WIFI_ROLE_CLIENT,
	WIFI_ROLE_AP,
};

struct wifi_sta {
	char ssid[WIFI_SSID_MAX_LEN + 1];
	enum wifi_security_type security;
	char pass[65];
	bool connected;
	uint8_t channel;
};

enum wifi_transport_type {
	WIFI_TRANSPORT_TCP,
	WIFI_TRANSPORT_UDP,
	WIFI_TRANSPORT_UDP_LITE,
	WIFI_TRANSPORT_TCP_SSL,
};

enum wifi_socket_state {
	WIFI_SOCKET_STATE_NONE,
	WIFI_SOCKET_STATE_CONNECTING,
	WIFI_SOCKET_STATE_CONNECTED,
};

struct wifi_off_socket {
	u8_t index;
	enum wifi_transport_type type;
	enum wifi_socket_state state;
	struct net_context *context;
	net_context_recv_cb_t recv_cb;
	net_context_connect_cb_t conn_cb;
	net_context_send_cb_t send_cb;
	void *user_data;
	struct net_pkt *tx_pkt;
	struct k_work connect_work;
	struct k_work send_work;
	struct k_delayed_work read_work;
	struct sockaddr peer_addr;
	struct k_sem read_sem;
};

struct wifi_dev {
	struct net_if *iface;
	struct wifi_bus_ops *bus;
	// struct wifi_gpio resetn;
	// struct wifi_gpio wakeup;
	scan_result_cb_t scan_cb;
	struct k_work_q work_q;
	struct k_work request_work;
	struct wifi_sta sta;
	enum wifi_request req;
	enum wifi_role role;
	u8_t mac[6];
	char buf[MAX_DATA_SIZE];
	struct k_mutex mutex;
	void *bus_data;
	struct wifi_off_socket socket[WIFI_OFFLOAD_MAX_SOCKETS];
}; */



















void wifi_iface_init(struct net_if *iface);
int wifi_mgmt_scan(struct device *dev, scan_result_cb_t cb);
int wifi_mgmt_connect(struct device *dev, struct wifi_connect_req_params *params);
int wifi_mgmt_disconnect(struct device *dev);
int wifi_mgmt_ap_enable(struct device *dev, struct wifi_connect_req_params *params);
int wifi_mgmt_ap_disable(struct device *dev);
