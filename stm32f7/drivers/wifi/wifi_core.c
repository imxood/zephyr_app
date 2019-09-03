
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zephyr.h>
#include <kernel.h>
#include <device.h>
#include <net/net_if.h>
#include <net/wifi_mgmt.h>
#include <net/net_offload.h>
#include <net/net_pkt.h>
#include <net/net_l2.h>
#include <net/net_context.h>
#include <net/net_offload.h>
#include <net/wifi_mgmt.h>
#include <net/socket.h>

#include "wifi.h"

#define WIFI_NAME "wifi"

static const struct net_wifi_mgmt_offload wifi_offload_api = {
	.iface_api.init = wifi_iface_init,
	.scan		= wifi_mgmt_scan,
	.connect	= wifi_mgmt_connect,
	.disconnect	= wifi_mgmt_disconnect,
	.ap_enable	= wifi_mgmt_ap_enable,
	.ap_disable	= wifi_mgmt_ap_disable,
};

NET_DEVICE_OFFLOAD_INIT(wifi_mgmt, WIFI_NAME,
			wifi_init, &wifi0, NULL,
			CONFIG_WIFI_INIT_PRIORITY, &wifi_offload_api, 1500);
