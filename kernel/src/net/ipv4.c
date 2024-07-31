//
// Created by ndraey on 01.11.23.
//

#include "net/ipv4.h"
#include "net/ethernet.h"
#include "net/endianess.h"
#include "io/ports.h"
#include "debug/hexview.h"
#include "lib/stdlib.h"
#include "mem/vmm.h"
#include "net/arp.h"
#include "net/udp.h"
#include "net/icmp.h"
#include "net/tcp.h"

void ipv4_handle_packet(netcard_entry_t *card, char *packet, size_t packet_size) {
	ETH_IPv4_PKG* ipv4_pkt = (ETH_IPv4_PKG*)packet;

	ipv4_pkt->TotalLength = ntohs(ipv4_pkt->TotalLength);

	qemu_warn("IPV4");
	qemu_log("  |--- Version: %x", ipv4_pkt->Version);
	qemu_log("  |--- DSF: %x", ipv4_pkt->DSF);
	qemu_log("  |--- TotalLength: %x", ipv4_pkt->TotalLength);
	qemu_log("  |--- ID: %x", ipv4_pkt->ID);
	qemu_log("  |--- Flags: %x", ipv4_pkt->Flags);
	qemu_log("  |--- TimeLife: %x", ipv4_pkt->TimeLife);
	qemu_log("  |--- Protocol: %x", ipv4_pkt->Protocol);
	qemu_log("  |--- Checksum: %x", ipv4_pkt->Checksum);
	qemu_log("  |--- Source: %d.%d.%d.%d", ipv4_pkt->Source[0], ipv4_pkt->Source[1], ipv4_pkt->Source[2], ipv4_pkt->Source[3]);
	qemu_log("  |--- Destination: %d.%d.%d.%d", ipv4_pkt->Destination[0], ipv4_pkt->Destination[1], ipv4_pkt->Destination[2], ipv4_pkt->Destination[3]);
	
	
	ethernet_frame_t* eth_frame = (packet - sizeof(ethernet_frame_t));
	qemu_log("  |--- Phys source: %x:%x:%x:%x:%x:%x",
			eth_frame->src_mac[0],
			eth_frame->src_mac[1],
			eth_frame->src_mac[2],
			eth_frame->src_mac[3],
			eth_frame->src_mac[4],
			eth_frame->src_mac[5]);

	// EXPERIMENTAL!
	arp_lookup_add(eth_frame->src_mac, ipv4_pkt->Source);

	if (ipv4_pkt->Protocol == ETH_IPv4_HEAD_UDP) {
		udp_handle_packet(card, (udp_packet_t *) (packet + sizeof(ETH_IPv4_PKG)));
	} else if(ipv4_pkt->Protocol == ETH_IPv4_HEAD_ICMPv4) {
		qemu_err("ICMP not implemented!");

		icmp_handle_packet(card, packet + sizeof(ETH_IPv4_PKG));
	} else if(ipv4_pkt->Protocol == ETH_IPv4_HEAD_TCP) {
       		qemu_note("HANDLING TCP!");

        	tcp_handle_packet(card, (tcp_packet_t*)(packet + sizeof(ETH_IPv4_PKG)));
	} else {
		qemu_log("  | |--- Header: [%x] %s", ipv4_pkt->Protocol, "Unknown");
		qemu_log("  | |--- RAW: %d bytes", packet_size - sizeof(ETH_IPv4_PKG));
		qemu_log("  | |");
	}
}

uint16_t ipv4_checksum(ETH_IPv4_PKG* packet) {
	// Treat the packet header as a 2-byte-integer array
	// Sum all integers up and flip all bits
	size_t array_size = sizeof(ETH_IPv4_PKG) / 2;
	uint16_t* array = (uint16_t*)packet;
	uint32_t sum = 0;

	for(int i = 0; i < array_size; i++) {
		sum += bit_flip_short(array[i]);
	}

	uint32_t carry = sum >> 16;
	sum = sum & 0x0000ffff;
	sum = sum + carry;

	return ~sum;
}

void ipv4_send_packet(netcard_entry_t *card, uint8_t dest_ip[4], const void *data, size_t size, uint8_t protocol) {
	ETH_IPv4_PKG* ipv4_pkt = kcalloc(1, sizeof(ETH_IPv4_PKG) + size);
	char* pkt_data = (char*)ipv4_pkt + sizeof(ETH_IPv4_PKG);

	qemu_log("IP send: %d bytes", size);

	ipv4_pkt->Version = 4;
	ipv4_pkt->HeaderLength = 5;
	memcpy(ipv4_pkt->Destination, dest_ip, 4);
	memcpy(ipv4_pkt->Source, card->ipv4_addr, 4);
	ipv4_pkt->Flags = 0; // No fragment
	ipv4_pkt->TimeLife = 64;
	ipv4_pkt->TotalLength = htons(sizeof(ETH_IPv4_PKG) + size);
	ipv4_pkt->Protocol = protocol;
	ipv4_pkt->Checksum = 0;
	ipv4_pkt->ID = 0;

	ipv4_pkt->Checksum = htons(ipv4_checksum(ipv4_pkt));

	memcpy(pkt_data, data, size);

	uint8_t dest_mac[6] = {0};
	uint8_t arp_zero[6] = {0};

	size_t spin = 10;

	while(!arp_lookup(dest_mac, dest_ip) && --spin) {
		arp_send_packet(card, arp_zero, dest_ip);
	}

	if(spin == 0) {
		qemu_err("%u.%u.%u.%u is unreachable", dest_ip[0], dest_ip[1], dest_ip[2], dest_ip[3]);

		goto end;
	}

	qemu_log("Total IP packet size: %d", sizeof(ETH_IPv4_PKG) + size);

	ethernet_send_packet(card, dest_mac, (uint8_t *) ipv4_pkt, sizeof(ETH_IPv4_PKG) + size, ETHERNET_TYPE_IPV4);

	end:

	kfree(ipv4_pkt);
}
