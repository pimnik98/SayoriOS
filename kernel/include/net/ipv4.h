//
// Created by ndraey on 01.11.23.
//

#pragma once

#include <common.h>
#include "net/cards.h"

#define IP_PROTOCOL_UDP 17

void ipv4_handle_packet(netcard_entry_t *card, char *packet, size_t packet_size);
void ipv4_send_packet(netcard_entry_t *card, uint8_t dest_ip[4], const void *data, size_t size, uint8_t protocol);