#pragma once

#include "common.h"

static void disk_linear_address_to_lba(size_t high_addr, size_t low_addr, size_t block_size, size_t* out_lba, size_t* out_offset) {
	size_t max_lba = 0xFFFFFFFF / block_size;

	*out_lba = (max_lba * high_addr) + (low_addr / block_size);
	*out_offset = low_addr % block_size;
}
