//
// Created by maractus on 03.01.24.
//

#pragma once

#include "common.h"

uint8_t ata_pio_read_sector(uint8_t drive, uint8_t *buf, uint32_t lba);
uint8_t ata_pio_write_raw_sector(uint8_t drive, const uint8_t *buf, uint32_t lba);

void ata_pio_write_sectors(uint8_t drive, uint8_t *buf, uint32_t lba, size_t sectors);
void ata_pio_read_sectors(uint8_t drive, uint8_t *buf, uint32_t lba, uint32_t numsects);
