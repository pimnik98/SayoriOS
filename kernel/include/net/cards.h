#pragma once

#include "common.h"

typedef struct {
    char name[64];
	uint8_t ipv4_addr[4];
    void (*get_mac_addr)(uint8_t[6]);
    void (*send_packet)(void*, size_t);
} netcard_entry_t;


void netcards_list_init();
void netcard_add(netcard_entry_t *card);
size_t netcards_get_count();
netcard_entry_t* netcard_get(size_t index);