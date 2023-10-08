#include "kernel.h"

void ethernet_send_packet(netcard_entry_t* card, uint8_t* dest_mac, uint8_t* data, size_t len, uint16_t type) {
    assert(card == 0, "%s", "Card is nullptr.");

    uint8_t src_mac[6];
    card->get_mac_addr(src_mac);
    
    ethernet_frame_t* frame = kmalloc(sizeof(ethernet_frame_t) + len);
    void* frame_data = (void*)frame + sizeof(ethernet_frame_t);

    memcpy(frame->src_mac, src_mac, 6);
    memcpy(frame->dest_mac, dest_mac, 6);
    memcpy(frame_data, data, len);

    frame->type = htons(type);

    card->send_packet(frame, sizeof(ethernet_frame_t) + len);

    kfree(frame);
}

void ethernet_handle_packet(ethernet_frame_t* packet, size_t len) {
    void* data = (void*)packet + sizeof(ethernet_frame_t);  // Get data
    size_t data_len = len - sizeof(ethernet_frame_t);  // Get length of data
    
    qemu_log("Received Ethernet Packet!");
    qemu_log("=> SRC[%v:%v:%v:%v:%v:%v]; DEST[%v:%v:%v:%v:%v:%v]; TYPE: %d", packet->src_mac, packet->dest_mac, packet->type);

	hexview_advanced(data, data_len, 10, true, new_qemu_printf);
}