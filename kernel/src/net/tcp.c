//
// Created by ndraey on 21.4.2024.
//

#include "net/ethernet.h"
#include "net/tcp.h"
#include "net/endianess.h"
#include "io/ports.h"

void tcp_handle_packet(netcard_entry_t *card, tcp_packet_t *packet) {
    qemu_note("!!!!!!!!!!!!!!!!!!!!!!!! TCP !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");

    ETH_IPv4_PKG *ipv4 = (ETH_IPv4_PKG *)((size_t)packet - sizeof(ETH_IPv4_PKG));
    size_t data_payload_size = ipv4->TotalLength - sizeof(ETH_IPv4_PKG);

    qemu_log("Data payload size: %d", data_payload_size);

    packet->source = ntohs(packet->source);
    packet->destination = ntohs(packet->destination);

    qemu_note("SRC: %d; DEST: %d", packet->source, packet->destination);

    qemu_note("FLAGS: SYN: %d; ACK: %d; PSH: %d; FIN: %d", packet->syn, packet->ack, packet->psh, packet->fin);
}
