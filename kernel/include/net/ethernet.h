#pragma once

#include "common.h"

#define ETHERNET_TYPE_ARP 0x0806
#define ETHERNET_TYPE_IP  0x0800

#define HARDWARE_TYPE_ETHERNET 0x01

typedef struct ethernet_frame {
  uint8_t dest_mac[6];
  uint8_t src_mac[6];
  uint16_t type;
  uint8_t data[];
} __attribute__((packed)) ethernet_frame_t;

void ethernet_send_packet(netcard_entry_t* card, uint8_t* dest_mac, uint8_t* data, size_t len, uint16_t type);
void ethernet_handle_packet(ethernet_frame_t* packet, size_t len);