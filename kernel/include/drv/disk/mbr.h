//
// Created by ndraey on 2/12/24.
//

#pragma once

#include "common.h"

struct mbr_parition {
    uint8_t activity;
    uint8_t start_head;
    uint8_t start_sector : 5;
    uint16_t start_cylinder : 10;
    uint8_t type;
    uint8_t end_head;
    uint8_t end_sector : 5;
    uint16_t end_cylinder : 10;
    uint32_t start_sector_lba;
    uint32_t num_sectors;
} __attribute__((packed));

void ebr_recursive_dump(char disk, uint64_t abs_lba, uint64_t lba, int depth);
void mbr_dump(char disk, uint64_t i);
