#pragma once

#include "common.h"
#include "cards.h"

#define ARP_REQUEST 1
#define ARP_REPLY 2

#define ARP_TABLE_MAX_SIZE 128

typedef struct arp_packet {
    uint16_t hardware_type;
    uint16_t protocol;
    uint8_t hardware_addr_len;
    uint8_t protocol_addr_len;
    uint16_t opcode;
    
    uint8_t src_mac[6];
    uint8_t src_ip[4];

    uint8_t dest_mac[6];
    uint8_t dest_ip[4];
} __attribute__((packed)) arp_packet_t;

typedef struct arp_table_entry {
    uint32_t ip_addr;
    uint8_t mac_addr[6];
} arp_table_entry_t;

void arp_handle_packet(netcard_entry_t* card, arp_packet_t* arp_packet, size_t len);
void arp_send_packet(netcard_entry_t* card, uint8_t* dst_hardware_addr, uint8_t* dst_protocol_addr);
bool arp_lookup(uint8_t* ret_hardware_addr, const uint8_t* ip_addr);
void arp_lookup_add(uint8_t* ret_hardware_addr, uint8_t* ip_addr);
void arp_init();
