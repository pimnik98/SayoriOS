//
// Created by maractus on 04.01.24.
//

#pragma once

#include "common.h"

void netstack_init();
void netstack_push(void* packet_data, size_t length);
void* netstack_pop();
void* netstack_poll();