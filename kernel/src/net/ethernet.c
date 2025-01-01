#include "net/ipv4.h"
#include "io/ports.h"
#include "net/endianess.h"
#include "net/arp.h"
#include "net/ethernet.h"
#include "mem/vmm.h"
//#include "debug/hexview.h"
#include "net/stack.h"

void ethernet_dump(void* data, size_t size, uint16_t type){
	qemu_log("Types: %x", type);

	char* pkg = (char*) data;

	if(pkg[0] == 0x45) {
		qemu_log("IPV4");
	} else if(pkg[0] == 0x60) {  // IPv6
		ETH_IPv6_PKG* ipv6 = (ETH_IPv6_PKG*)pkg;

		qemu_log("[NET] [DUMP] Определен: IPv6");
		qemu_log("  |--- Version: %x",ipv6->Version);
		qemu_log("  |--- Flow: %x %x %x",ipv6->Flow[0],ipv6->Flow[1],ipv6->Flow[2]);
		qemu_log("  |--- PayLoad: %x",ipv6->PayLoad);
		qemu_log("  |--- NextHead: %x",ipv6->NextHead);
		
		if (ipv6->NextHead == ETH_IPv6_HEAD_ICMPv6){
			ETH_ICMPv6_PKG* icmpv6 = (ETH_ICMPv6_PKG*)(pkg + sizeof(ETH_IPv6_PKG));

			qemu_log("  | |--- Header: [%x] %s", ipv6->NextHead, "ICMPv6");
			qemu_log("  | |--- Type: %x",icmpv6->Type);
			qemu_log("  | |--- Code: %x",icmpv6->Code);
			qemu_log("  | |--- CheckSum: %x",icmpv6->CheckSum);
			qemu_log("  | |--- RAW: %d bytes", size - sizeof(ETH_IPv6_PKG)  - sizeof(ETH_ICMPv6_PKG));
			qemu_log("  | |");

		} else if (ipv6->NextHead == ETH_IPv6_HEAD_UDP){
			ETH_UDP_PKG* udp = (ETH_UDP_PKG*)(pkg + sizeof(ETH_IPv6_PKG));
			qemu_log("  | |--- Header: [%x] %s", ipv6->NextHead, "UDP");
			qemu_log("  | |--- SourcePort: %d",udp->SourcePort);
			qemu_log("  | |--- DestinationPort: %d",udp->DestinationPort);
			qemu_log("  | |--- Length: %d",udp->Length);
			qemu_log("  | |--- CheckSum: %x",udp->CheckSum);
			qemu_log("  | |--- RAW: %d bytes", size - sizeof(ETH_IPv6_PKG)  - sizeof(ETH_UDP_PKG));
			qemu_log("  | |");

		} else {
			qemu_log("  | |--- Header: [%x] %s", ipv6->NextHead, "Unknown");
			qemu_log("  | |--- RAW: %d bytes", size - sizeof(ETH_IPv6_PKG));
			qemu_log("  | |");
		}

		ipv6->Source[0] = htons(ipv6->Source[0]);
		ipv6->Source[1] = htons(ipv6->Source[1]);
		ipv6->Source[2] = htons(ipv6->Source[2]);
		ipv6->Source[3] = htons(ipv6->Source[3]);
		ipv6->Source[4] = htons(ipv6->Source[4]);
		ipv6->Source[5] = htons(ipv6->Source[5]);
		ipv6->Source[6] = htons(ipv6->Source[6]);
		ipv6->Source[7] = htons(ipv6->Source[7]);

		ipv6->Destination[0] = htons(ipv6->Destination[0]);
		ipv6->Destination[1] = htons(ipv6->Destination[1]);
		ipv6->Destination[2] = htons(ipv6->Destination[2]);
		ipv6->Destination[3] = htons(ipv6->Destination[3]);
		ipv6->Destination[4] = htons(ipv6->Destination[4]);
		ipv6->Destination[5] = htons(ipv6->Destination[5]);
		ipv6->Destination[6] = htons(ipv6->Destination[6]);
		ipv6->Destination[7] = htons(ipv6->Destination[7]);

		qemu_log("  |--- HopLimit: %x",ipv6->HopLimit);
		qemu_log("  |--- Source: %x:%x:%x:%x:%x:%x:%x:%x", ipv6->Source[0], ipv6->Source[1], ipv6->Source[2], ipv6->Source[3], ipv6->Source[4], ipv6->Source[5], ipv6->Source[6], ipv6->Source[7]);
		qemu_log("  |--- Destination: %x:%x:%x:%x:%x:%x:%x:%x", ipv6->Destination[0], ipv6->Destination[1], ipv6->Destination[2], ipv6->Destination[3], ipv6->Destination[4], ipv6->Destination[5], ipv6->Destination[6], ipv6->Destination[7]);
		/*
		qemu_log("  |--- Дополнительная информация");
		qemu_log("    |--- Type: %d",ipv6->Opt.Type);
		qemu_log("    |--- Size: %d",ipv6->Opt.Size);

		// First two bytes may indicate a checksum according to Wireshark
		qemu_log("    |--- Data: %x:%x:%x:%x:%x:%x", ipv6->Opt.MAC[0], ipv6->Opt.MAC[1], ipv6->Opt.MAC[2], ipv6->Opt.MAC[3], ipv6->Opt.MAC[4], ipv6->Opt.MAC[5]);
		*/
	} else {
		qemu_log("UNKNOWN ETHERNET PACKET TYPE");
	}
}

void ethernet_send_packet(netcard_entry_t* card, uint8_t* dest_mac, uint8_t* data, size_t len, uint16_t type) {
    assert(card == 0, "%s", "Card is nullptr.");

    uint8_t src_mac[6];
    card->get_mac_addr(src_mac);
    
    ethernet_frame_t* frame = kmalloc(sizeof(ethernet_frame_t) + len);
    void* frame_data = (char*)frame + sizeof(ethernet_frame_t);

    memcpy(frame->src_mac, src_mac, 6);
    memcpy(frame->dest_mac, dest_mac, 6);
    memcpy(frame_data, data, len);

    frame->type = htons(type);

    netstack_push(card, frame, sizeof(ethernet_frame_t) + len);
    //card->send_packet(frame, sizeof(ethernet_frame_t) + len);

    kfree(frame);
}

void ethernet_handle_packet(netcard_entry_t *card, ethernet_frame_t *packet, size_t len) {
    void* data = (void*)packet + sizeof(ethernet_frame_t);  // Get data
    size_t data_len = len - sizeof(ethernet_frame_t);  // Get length of data
    
    qemu_log("Received Ethernet Packet!");
    qemu_log("=> SRC[%x:%x:%x:%x:%x:%x]; DEST[%x:%x:%x:%x:%x:%x]; TYPE: %x",
			 packet->src_mac[0],
			 packet->src_mac[1],
			 packet->src_mac[2],
			 packet->src_mac[3],
			 packet->src_mac[4],
			 packet->src_mac[5],
			 packet->dest_mac[0],
			 packet->dest_mac[1],
			 packet->dest_mac[2],
			 packet->dest_mac[3],
			 packet->dest_mac[4],
			 packet->dest_mac[5],
			 bit_flip_short(packet->type)
		 );

//	hexview_advanced(data, data_len, 10, true, new_qemu_printf);

	ethernet_dump(data, data_len,bit_flip_short(packet->type));

	if(ntohs(packet->type) == ETHERNET_TYPE_ARP) {
		arp_handle_packet(card, data, data_len);
	} else if(ntohs(packet->type) == ETHERNET_TYPE_IPV4) {
		ipv4_handle_packet(card, data, data_len);
	}
}
