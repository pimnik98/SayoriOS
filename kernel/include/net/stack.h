//
// Created by maractus on 04.01.24.
//

#pragma once

#include "common.h"
#include "net/cards.h"

typedef struct {
	netcard_entry_t* card;
	void* data;
	size_t length;
} netqueue_item_t;

void netstack_init();
void netstack_push(netcard_entry_t* card, void* packet_data, size_t length);
netqueue_item_t* netstack_pop();
netqueue_item_t* netstack_poll();
