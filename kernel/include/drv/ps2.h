//
// Created by ndraey on 2/15/24.
//

#pragma once

#include "common.h"

#define		PS2_DATA_PORT		0x60
#define		PS2_STATE_REG		0x64

uint8_t ps2_read();
void ps2_write(uint8_t byte);

uint8_t ps2_read_configuration_byte();
void ps2_write_configuration_byte(uint8_t byte);

void ps2_in_wait_until_empty();
void ps2_out_wait_until_full();

void ps2_init();