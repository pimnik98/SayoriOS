//
// Created by ndraey on 21.4.2024.
//

#include "common.h"
#include "net/cards.h"
#include "net/ethernet.h"
#include "net/tcp.h"
#include "net/endianess.h"
#include "io/ports.h"
#include "net/ipv4.h"

#define MAX_CONNECTIONS 64

tcp_connection_t tcp_connections[MAX_CONNECTIONS] = {};

int tcp_find_connection(uint8_t address[4], size_t port) {
	for(int i = 0; i < MAX_CONNECTIONS; i++) {
		if(memcmp((uint8_t*)&tcp_connections[i].dest_ip_addr, address, 4) == 0
			&& tcp_connections[i].source_port == port
			&& tcp_connections[i].status != TCP_NONE) {
			return i;
		}
	}

	return -1;
}

bool tcp_new_connection(netcard_entry_t* card, uint8_t address[4], size_t port, size_t seq_nr) {
	uint8_t empty_addr[4] = {0, 0, 0, 0};
	int index = -1;

	for(int i = 0; i < MAX_CONNECTIONS; i++) {
		if(memcmp((uint8_t*)&tcp_connections[i].dest_ip_addr, empty_addr, 4) == 0
				&& tcp_connections[i].source_port == 0
				&& tcp_connections[i].status == TCP_NONE) {
			index = i;
			break;
		}
	}

	if(index == -1) {
		return false;
	}

	memcpy(&tcp_connections[index].dest_ip_addr, address, 4);
	tcp_connections[index].source_port = port;
	tcp_connections[index].status = TCP_CREATED;
	tcp_connections[index].seq = seq_nr;
	tcp_connections[index].card = card;

	return true;
}

void tcp_handle_packet(netcard_entry_t *card, tcp_packet_t *packet) {
	qemu_note("!!!!!!!!!!!!!!!!!!!!!!!! TCP !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");

	ETH_IPv4_PKG *ipv4 = (ETH_IPv4_PKG *)((size_t)packet - sizeof(ETH_IPv4_PKG));
	size_t data_payload_size = ipv4->TotalLength - sizeof(ETH_IPv4_PKG);

	qemu_log("Data payload size: %d", data_payload_size);

	packet->source = ntohs(packet->source);
	packet->destination = ntohs(packet->destination);

	qemu_log("FROM: %u.%u.%u.%u", ipv4->Source[0], ipv4->Source[1], ipv4->Source[2], ipv4->Source[3]);

	qemu_note("SRC: %d; DEST: %d", packet->source, packet->destination);

	qemu_note("FLAGS: SYN: %d; ACK: %d; PSH: %d; FIN: %d", packet->syn, packet->ack, packet->psh, packet->fin);

	packet->ack_seq = ntohl(packet->ack_seq);
	packet->seq = ntohl(packet->seq);

	
	int idx = -1;
	if(tcp_find_connection(ipv4->Source, packet->source) == -1) {
		tcp_new_connection(card, ipv4->Source, packet->source, packet->seq);
		qemu_ok("Created new connection!");
	}
	idx = tcp_find_connection(ipv4->Source, packet->source);

	qemu_note("Connection idx: %d", idx);

	bool is_stage_1 = packet->syn && !packet->ack && !packet->psh && !packet->fin;
	bool is_stage_2 = !packet->syn && packet->ack && !packet->psh && !packet->fin;
	bool is_push = !packet->syn && !packet->ack && packet->psh && !packet->fin;


	tcp_packet_t sendable_packet = {};
	memcpy(&sendable_packet, packet, sizeof(tcp_packet_t));
	
	if(is_stage_1) {
		tcp_connections[idx].seq = rand();
		tcp_connections[idx].ack = sendable_packet.seq + 1;

		sendable_packet.ack = 1;
		sendable_packet.seq = ntohl(tcp_connections[idx].seq);  // it's rand();
		sendable_packet.ack_seq = ntohl(tcp_connections[idx].ack);

		uint16_t dest = ntohs(sendable_packet.destination);
		uint16_t src = ntohs(sendable_packet.source);
		sendable_packet.source = dest;
		sendable_packet.destination = src;

		sendable_packet.doff = 5;

		ipv4_send_packet(tcp_connections[idx].card, ipv4->Source, &sendable_packet, sizeof(tcp_packet_t), ETH_IPv4_HEAD_TCP);
	}
}
