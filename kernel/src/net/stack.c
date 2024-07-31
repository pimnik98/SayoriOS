//
// Created by maractus on 04.01.24.
//

#include "net/stack.h"
#include "../lib/libvector/include/vector.h"
#include "mem/vmm.h"

volatile vector_t* system_network_incoming_queue = 0;
volatile vector_t* system_network_outgoing_queue = 0;

void netstack_init() {
    system_network_incoming_queue = vector_new();
    system_network_outgoing_queue = vector_new();
}

void netstack_push(void* packet_data, size_t length) {
    void* data = kcalloc(1, length);

    memcpy(data, packet_data, length);

    vector_push_back(system_network_stack, (size_t) data);
}

void* netstack_pop() {
    void* data = (void *) vector_pop_back(system_network_stack).element;

    return data;
}

void* netstack_poll() {
    while(system_network_stack->size == 0);

    return netstack_pop();
}
