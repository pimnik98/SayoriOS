#include "kernel.h"

uint8_t default_broadcast_mac_address[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

arp_table_entry_t arp_table[ARP_TABLE_MAX_SIZE];
size_t arp_table_size;
size_t arp_table_curr;

void arp_handle_packet(netcard_entry_t* card, arp_packet_t* arp_packet, size_t len) {
    uint8_t dest_mac[6];
    uint8_t dest_ip[4];
    
    memcpy(dest_mac, arp_packet->src_mac, 6);
    memcpy(dest_ip, arp_packet->src_ip, 4);
    
    if(ntohs(arp_packet->opcode) == ARP_REQUEST) {
        qemu_log("ARP REQUEST");

        uint32_t my_ip = 0x0e02000a; // 10.0.2.14
        if(memcmp((const char*)arp_packet->dest_ip, (const char*)&my_ip, 4) != 0) {
            // Set source MAC address, IP address (hardcode the IP address as 10.2.2.3 until we really get one...)
            card->get_mac_addr(arp_packet->src_mac);
            
            arp_packet->src_ip[0] = 10;
            arp_packet->src_ip[1] = 0;
            arp_packet->src_ip[2] = 2;
            arp_packet->src_ip[3] = 14;

            // Set destination MAC address, IP address
            memcpy(arp_packet->dest_mac, dest_mac, 6);
            memcpy(arp_packet->dest_ip, dest_ip, 4);

            // Set opcode
            arp_packet->opcode = htons(ARP_REPLY);

            // Set lengths
            arp_packet->hardware_addr_len = 6;
            arp_packet->protocol_addr_len = 4;

            // Set hardware type
            arp_packet->hardware_type = htons(HARDWARE_TYPE_ETHERNET);

            // Set protocol = IPv4
            arp_packet->protocol = htons(ETHERNET_TYPE_IP);

            // Now send it with ethernet
            ethernet_send_packet(card, dest_mac, (uint8_t*)arp_packet, sizeof(arp_packet_t), ETHERNET_TYPE_ARP);
        }
    } else if(ntohs(arp_packet->opcode) == ARP_REPLY){
        qemu_log("ARP REPLY");    

        qemu_log("Source IP: ", arp_packet->src_ip);
        qemu_log("Source MAC: ", arp_packet->src_mac);
        qemu_log("Destination IP: ", arp_packet->dest_ip);
        qemu_log("Destination MAC: ", arp_packet->dest_mac);
    } else {
        qemu_printf("Got unknown ARP (%d)\n", arp_packet->opcode);
    }
 
    // Now, store the ip-mac address mapping relation
    memcpy(&arp_table[arp_table_curr].ip_addr, dest_ip, 4);
    memcpy(&arp_table[arp_table_curr].mac_addr, dest_mac, 6);

    if(arp_table_size < ARP_TABLE_MAX_SIZE)
        arp_table_size++;

    if(arp_table_curr >= ARP_TABLE_MAX_SIZE)
        arp_table_curr = 0;
}

void arp_send_packet(netcard_entry_t* card, uint8_t* dest_mac, uint8_t* dest_ip) {
    arp_packet_t* arp_packet = kmalloc(sizeof(arp_packet_t));

    // Set source MAC address, IP address (hardcode the IP address as 10.2.2.3 until we really get one..)
    card->get_mac_addr(arp_packet->src_mac);
    
    arp_packet->src_ip[0] = 10;
    arp_packet->src_ip[1] = 0;
    arp_packet->src_ip[2] = 2;
    arp_packet->src_ip[3] = 14;

    // Set destination MAC address, IP address
    memcpy(arp_packet->dest_mac, dest_mac, 6);
    memcpy(arp_packet->dest_ip, dest_ip, 4);

    // Set opcode
    arp_packet->opcode = htons(ARP_REQUEST);

    // Set lengths
    arp_packet->hardware_addr_len = 6;
    arp_packet->protocol_addr_len = 4;

    // Set hardware type
    arp_packet->hardware_type = htons(HARDWARE_TYPE_ETHERNET);

    // Set protocol = IPv4
    arp_packet->protocol = htons(ETHERNET_TYPE_IP);

    // Now send it with ethernet
    ethernet_send_packet(card, default_broadcast_mac_address, (uint8_t*)arp_packet, sizeof(arp_packet_t), ETHERNET_TYPE_ARP);
}

void arp_lookup_add(uint8_t * ret_hardware_addr, uint8_t * ip_addr) {
    memcpy(&arp_table[arp_table_curr].ip_addr, ip_addr, 4);
    memcpy(&arp_table[arp_table_curr].mac_addr, ret_hardware_addr, 6);
    
    if(arp_table_size < ARP_TABLE_MAX_SIZE)
        arp_table_size++;
    
    if(arp_table_curr >= ARP_TABLE_MAX_SIZE)
        arp_table_curr = 0;
}

bool arp_lookup(uint8_t* ret_hardware_addr, const uint8_t* ip_addr) {
    uint32_t ip_entry = *((uint32_t*)(ip_addr));
    
    for(int i = 0; i < ARP_TABLE_MAX_SIZE; i++) {
        if(arp_table[i].ip_addr == ip_entry) {
            memcpy(ret_hardware_addr, &arp_table[i].mac_addr, 6);
            return true;
        }
    }

    return false;
}

void arp_init() {
    uint8_t broadcast_ip[4];
    uint8_t broadcast_mac[6];

    memset(broadcast_ip, 0xff, 4);
    memset(broadcast_mac, 0xff, 6);
 
    arp_lookup_add(broadcast_mac, broadcast_ip);
}