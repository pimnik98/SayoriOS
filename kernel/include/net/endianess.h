#pragma once
#include "common.h"

uint8_t bit_flip_byte(uint8_t byte, int num_bits);
uint16_t bit_flip_short(uint16_t short_int);
uint32_t bit_flip_int(uint32_t long_int);
uint8_t htonb(uint8_t byte, int num_bits);
uint8_t ntohb(uint8_t byte, int num_bits);
uint16_t htons(uint16_t hostshort);
uint32_t htonl(uint32_t hostlong);
uint16_t ntohs(uint16_t netshort);
uint32_t ntohl(uint32_t netlong);