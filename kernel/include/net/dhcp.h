#pragma once

#include "common.h"

#define DHCP_REQUEST 1
#define DHCP_REPLY 2

typedef struct dhcp_packet {
	uint8_t op;
	uint8_t hardware_type;
	uint8_t hardware_addr_len;
	uint8_t hops;
	uint32_t xid;
	uint16_t seconds;
	uint16_t flags;
	uint32_t client_ip;
	uint32_t your_ip;
	uint32_t server_ip;
	uint32_t gateway_ip;
	uint8_t client_hardware_addr[16];
	uint8_t server_name[64];
	uint8_t file[128];
	uint8_t options[340];
} __attribute__ ((packed)) dhcp_packet_t;

void dhcp_discover(netcard_entry_t* card);
void dhcp_handle_packet(netcard_entry_t* card, dhcp_packet_t* packet);
void dhcp_request(netcard_entry_t* card, const uint8_t req_ip[4]);