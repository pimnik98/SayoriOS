#pragma once

#include "common.h"

void mtrr_init();
void read_mtrr(size_t index, uint32_t* base, uint32_t* mask);
void write_mtrr(size_t index, uint32_t base, uint32_t mask);
void write_mtrr_size(size_t index, uint32_t base, uint32_t size, size_t type);
size_t find_free_mtrr();