#include "net/udp.h"
#include "mem/vmm.h"
#include "net/endianess.h"
#include "io/ports.h"
#include "net/ipv4.h"
#include "debug/hexview.h"
#include "net/dhcp.h"

uint16_t udp_calculate_checksum(udp_packet_t * packet) {
	// UDP checksum is optional in IPv4
	return 0;
}

void udp_send_packet(netcard_entry_t* card, uint8_t * dst_ip, uint16_t src_port, uint16_t dst_port, void * data, int len) {
	size_t length = sizeof(udp_packet_t) + len;

	udp_packet_t* packet = kcalloc(1, length);

	packet->src_port = htons(src_port);
	packet->dst_port = htons(dst_port);
	packet->length = htons(length);
	packet->checksum = udp_calculate_checksum(packet);

	memcpy((char*)packet + sizeof(udp_packet_t), data, len);

	qemu_log("UDP Packet sent");
	ipv4_send_packet(card, dst_ip, packet, length, IP_PROTOCOL_UDP);
}

void udp_handle_packet(netcard_entry_t *card, udp_packet_t *packet) {
	uint16_t src_port = ntohs(packet->src_port);
	uint16_t dst_port = ntohs(packet->dst_port);
	uint16_t length = ntohs(packet->length);

	void* data_ptr = (char*)packet + sizeof(udp_packet_t);

	qemu_log("UDP: Source port: %d; Destination port: %d; Length: %d", src_port, dst_port, length);

//	hexview_advanced(data_ptr, data_len, 16, true, new_qemu_printf);

	if(dst_port == 68) {
		dhcp_handle_packet(card, data_ptr);
	}
}