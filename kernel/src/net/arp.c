#include "common.h"
#include "net/arp.h"
#include "net/cards.h"
#include "mem/vmm.h"
#include "net/endianess.h"
#include "lib/string.h"
#include "io/ports.h"
#include "net/ethernet.h"

uint8_t default_broadcast_mac_address[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

arp_table_entry_t arp_table[ARP_TABLE_MAX_SIZE] = {0};
size_t arp_table_curr = 0;

void arp_handle_packet(netcard_entry_t* card, arp_packet_t* arp_packet, size_t len) {
    uint8_t dest_mac[6];
    uint8_t dest_ip[4];
    
    memcpy(dest_mac, arp_packet->src_mac, 6);
    memcpy(dest_ip, arp_packet->src_ip, 4);
    
    if(ntohs(arp_packet->opcode) == ARP_REQUEST) {
        qemu_warn("ARP REQUEST");

        if(memcmp((const char*)arp_packet->dest_ip, (const char*)card->ipv4_addr, 4) != 0) {
            // Set source MAC address, IP address (hardcode the IP address as 10.2.2.3 until we really get one...)
            card->get_mac_addr(arp_packet->src_mac);
            
            arp_packet->src_ip[0] = card->ipv4_addr[0];
            arp_packet->src_ip[1] = card->ipv4_addr[1];
            arp_packet->src_ip[2] = card->ipv4_addr[2];
            arp_packet->src_ip[3] = card->ipv4_addr[3];

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
            arp_packet->protocol = htons(ETHERNET_TYPE_IPV4);

            // Now send it with ethernet
            ethernet_send_packet(card, dest_mac, (uint8_t*)arp_packet, sizeof(arp_packet_t), ETHERNET_TYPE_ARP);
        }
	} else if(ntohs(arp_packet->opcode) == ARP_REPLY){
		qemu_log("ARP REPLY");    

		qemu_log("Source IP: %d.%d.%d.%d", arp_packet->src_ip[0], arp_packet->src_ip[1], arp_packet->src_ip[2], arp_packet->src_ip[3]);
		qemu_log("Source MAC: %x:%x:%x:%x:%x:%x",
					 arp_packet->src_mac[0],
					 arp_packet->src_mac[1],
					 arp_packet->src_mac[2],
					 arp_packet->src_mac[3],
					 arp_packet->src_mac[4],
					 arp_packet->src_mac[5]);
		qemu_log("Destination IP: %d.%d.%d.%d",
				arp_packet->dest_ip[0],
				arp_packet->dest_ip[1],
				arp_packet->dest_ip[2],
				arp_packet->dest_ip[3]);
		qemu_log("Destination MAC: %x:%x:%x:%x:%x:%x",
					 arp_packet->dest_mac[0],
					 arp_packet->dest_mac[1],
					 arp_packet->dest_mac[2],
					 arp_packet->dest_mac[3],
					 arp_packet->dest_mac[4],
					 arp_packet->dest_mac[5]);
	    } else {
		qemu_log("Got unknown ARP opcode (%d)", arp_packet->opcode);
	    }
 
    // Now, store the ip-mac address mapping relation
    
	arp_lookup_add(dest_mac, dest_ip);

}

void arp_send_packet(netcard_entry_t* card, uint8_t* dest_mac, uint8_t* dest_ip) {
    arp_packet_t* arp_packet = kcalloc(sizeof(arp_packet_t), 1);

    card->get_mac_addr(arp_packet->src_mac);
    
    arp_packet->src_ip[0] = card->ipv4_addr[0];
    arp_packet->src_ip[1] = card->ipv4_addr[1];
    arp_packet->src_ip[2] = card->ipv4_addr[2];
    arp_packet->src_ip[3] = card->ipv4_addr[3];

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
    arp_packet->protocol = htons(ETHERNET_TYPE_IPV4);

    // Now send it with ethernet
    ethernet_send_packet(
			card,
			default_broadcast_mac_address,
			(uint8_t*)arp_packet,
			sizeof(arp_packet_t),
			ETHERNET_TYPE_ARP);
}

void arp_lookup_add(uint8_t* ret_hardware_addr, uint8_t * ip_addr) {
    	qemu_note("lookup add: [%x:%x:%x:%x:%x:%x] is [%d.%d.%d.%d]", 
			ret_hardware_addr[0],
			ret_hardware_addr[1],
			ret_hardware_addr[2],
			ret_hardware_addr[3],
			ret_hardware_addr[4],
			ret_hardware_addr[5],
			ip_addr[0],
			ip_addr[1],
			ip_addr[2],
			ip_addr[3]);

	memcpy(&arp_table[arp_table_curr].ip_addr, ip_addr, 4);
	memcpy(&arp_table[arp_table_curr].mac_addr, ret_hardware_addr, 6);
    
	if(arp_table_curr < ARP_TABLE_MAX_SIZE) {
		arp_table_curr++;
	}
    
	if(arp_table_curr >= ARP_TABLE_MAX_SIZE) {
		arp_table_curr = 0;
	}
}

bool arp_lookup(uint8_t* ret_hardware_addr, const uint8_t* ip_addr) {
    uint32_t ip_entry = *((uint32_t*)(ip_addr));

	qemu_log("Looking up: %d.%d.%d.%d", ip_addr[0], ip_addr[1], ip_addr[2], ip_addr[3]);

    for(int i = 0; i < ARP_TABLE_MAX_SIZE; i++) {
        if(arp_table[i].ip_addr == ip_entry) {
            memcpy(ret_hardware_addr, arp_table[i].mac_addr, 6);
			qemu_ok("Found %x:%x:%x:%x:%x:%x!",
						arp_table[i].mac_addr[0],
						arp_table[i].mac_addr[1],
						arp_table[i].mac_addr[2],
						arp_table[i].mac_addr[3],
						arp_table[i].mac_addr[4],
						arp_table[i].mac_addr[5]);
            return true;
        }
    }

	qemu_err("Failed!");

    return false;
}

void arp_init() {
    uint8_t broadcast_ip[4];
    uint8_t broadcast_mac[6];

    memset(broadcast_ip, 0xff, 4);
    memset(broadcast_mac, 0xff, 6);
 
    arp_lookup_add(broadcast_mac, broadcast_ip);
}
