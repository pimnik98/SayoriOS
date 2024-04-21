//
// Created by ndraey on 21.4.2024.
//

#include "net/tcp.h"
#include "net/endianess.h"
#include "io/ports.h"

void tcp_handle_packet(netcard_entry_t *card, tcp_packet_t *packet) {
    qemu_note("!!!!!!!!!!!!!!!!!!!!!!!! TCP !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");

    packet->source = ntohs(packet->source);
    packet->destination = ntohs(packet->destination);

    qemu_note("SRC: %d; DEST: %d", packet->source, packet->destination);

    qemu_note("FLAGS: SYN: %d; ACK: %d; PSH: %d; FIN: %d", packet->syn, packet->ack, packet->psh, packet->fin);
}