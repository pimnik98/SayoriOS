#pragma once

#include "common.h"
#include "net/cards.h"

typedef struct udp_packet {
	uint16_t src_port;
	uint16_t dst_port;
	uint16_t length;
	uint16_t checksum;
} __attribute__((packed)) udp_packet_t;

void udp_send_packet(netcard_entry_t* card, uint8_t * dst_ip, uint16_t src_port, uint16_t dst_port, void * data, int len);
void udp_handle_packet(netcard_entry_t *card, udp_packet_t *packet);