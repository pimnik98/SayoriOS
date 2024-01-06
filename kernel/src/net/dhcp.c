#include "net/cards.h"
#include "net/dhcp.h"
#include "net/endianess.h"
#include "mem/vmm.h"
#include "lib/rand.h"
#include "net/udp.h"
#include "io/ports.h"

void dhcp_discover(netcard_entry_t* card) {
	// DHCP-клиент должен быть готов принять DHCP-сообщение длиной в 576 байт.

	dhcp_packet_t* packet = kcalloc(sizeof(dhcp_packet_t), 1);

	uint8_t card_mac[6] = {0};
	card->get_mac_addr(card_mac);

	packet->op = DHCP_REQUEST;
	packet->hardware_type = 0x01;  // Ethernet
	packet->hardware_addr_len = 0x06;  // Mac address is 6 bytes long
	packet->hops = 0;
	packet->xid = ntohl((size_t)rand());
	packet->seconds = 0;
	packet->flags = htons(0x8000);
	packet->client_ip = 0;
	packet->your_ip = 0;
	packet->server_ip = 0;
	packet->gateway_ip = 0;

	memcpy(packet->client_hardware_addr, card_mac, 6);

	uint8_t* options_field = packet->options;

	*((uint32_t*)options_field) = htonl(0x63825363);

	// Discover DHCP
	options_field[4] = 53;
	options_field[5] = 1;
	options_field[6] = 1;

	// Address resolution
	options_field[7] = 50;
	options_field[8] = 4;

	// Last known IP-Address
	options_field[9] = 0;
	options_field[10] = 0;
	options_field[11] = 0;
	options_field[12] = 0;

	// Host name
	options_field[13] = 12;
	options_field[14] = 7;
	options_field[15] = 'S';
	options_field[16] = 'a';
	options_field[17] = 'y';
	options_field[18] = 'o';
	options_field[19] = 'r';
	options_field[20] = 'i';
	options_field[21] = 0;

	// End
	options_field[22] = 0xff;

	uint8_t dest_ip[4] = {0xff, 0xff, 0xff, 0xff};

	qemu_log("DHCP size: %d", sizeof(dhcp_packet_t));
	udp_send_packet(card, dest_ip, 68, 67, packet, sizeof(dhcp_packet_t));

	kfree(packet);
}

void dhcp_request(netcard_entry_t* card, const uint8_t req_ip[4]) {
	dhcp_packet_t* packet = kcalloc(sizeof(dhcp_packet_t), 1);

	uint8_t card_mac[6] = {0};
	card->get_mac_addr(card_mac);

	packet->op = DHCP_REQUEST;
	packet->hardware_type = 0x01;  // Ethernet
	packet->hardware_addr_len = 0x06;  // Mac address is 6 bytes long
	packet->hops = 0;
	packet->xid = ntohl((size_t)rand());
	packet->seconds = 0;
	packet->flags = htons(0x8000);
	packet->client_ip = 0;
	packet->your_ip = 0;
	packet->server_ip = 0;
	packet->gateway_ip = 0;

	memcpy(packet->client_hardware_addr, card_mac, 6);

	uint8_t* options_field = packet->options;

	*((uint32_t*)options_field) = htonl(0x63825363);

	// Discover DHCP
	options_field[4] = 53;
	options_field[5] = 1;
	options_field[6] = 3;

	// Address resolution
	options_field[7] = 50;
	options_field[8] = 4;

	// Last known IP-Address
	options_field[9] = req_ip[0];
	options_field[10] = req_ip[1];
	options_field[11] = req_ip[2];
	options_field[12] = req_ip[3];

	// Host name
	options_field[13] = 12;
	options_field[14] = 7;
	options_field[15] = 'S';
	options_field[16] = 'a';
	options_field[17] = 'y';
	options_field[18] = 'o';
	options_field[19] = 'r';
	options_field[20] = 'i';
	options_field[21] = 0;

	// End
	options_field[22] = 0xff;

	uint8_t dest_ip[4] = {0xff, 0xff, 0xff, 0xff};
//	udp_send_packet(card, dest_ip, 68, 67, packet, 576);

	qemu_log("DHCP size: %d", sizeof(dhcp_packet_t));
	udp_send_packet(card, dest_ip, 68, 67, packet, sizeof(dhcp_packet_t));

	kfree(packet);
}

void dhcp_handle_packet(netcard_entry_t* card, dhcp_packet_t* packet) {
	uint8_t* options = packet->options + 4;

	char* my_ip = (char*)&packet->your_ip;
	char* sv_ip = (char*)&packet->server_ip;

	qemu_log("DHCP");
	qemu_log("Operation: %d", packet->op);
	qemu_log("Client IP: %x", packet->client_ip);
	qemu_log("Your IP: %x (%d.%d.%d.%d)", packet->your_ip, (unsigned char)my_ip[0], (unsigned char)my_ip[1], (unsigned char)my_ip[2], (unsigned char)my_ip[3]);
	qemu_log("Server IP: %x (%d.%d.%d.%d)", packet->server_ip, (unsigned char)sv_ip[0], (unsigned char)sv_ip[1], (unsigned char)sv_ip[2], (unsigned char)sv_ip[3]);
	qemu_log("Gateway IP: %x", packet->gateway_ip);

	size_t msgtype = *options;
	qemu_log("First option is: %d", *options);

	if(msgtype == 53) {
		uint8_t type = *(options + 2);

		if(type == 2) {
			qemu_ok("Request!");
			// Does not work because it executes in IRQ
//			dhcp_request(card, (uint8_t *) &packet->your_ip);
		} else if(type == 5) {
			memcpy(card->ipv4_addr, my_ip, 4);

			qemu_ok("Acquired an IP address: %u.%u.%u.%u", card->ipv4_addr[0], card->ipv4_addr[1], card->ipv4_addr[2], card->ipv4_addr[3]);
		}

		qemu_log("Type: %d", type);
	}
}

void dhcp_init_all_cards() {
	for(int i = 0; i < netcards_get_count(); i++) {
		netcard_entry_t* card = netcard_get(i);

		qemu_log("Initializing DHCP for: %s", card->name);

		dhcp_discover(card);
		dhcp_request(card, card->ipv4_addr);
	}
}
