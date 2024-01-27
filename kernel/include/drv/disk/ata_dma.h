#pragma once

#include "common.h"
#include "sys/status.h"

#define ATA_DMA_MARK_END 0x8000

typedef struct prdt {
	uint32_t buffer_phys;
	uint16_t transfer_size;
	uint16_t mark_end;
} __attribute__((packed)) prdt_t;

void ata_dma_init();
status_t ata_dma_read_sectors(uint8_t drive, uint8_t *buf, uint32_t lba, uint8_t numsects);
status_t ata_dma_write_sectors(uint8_t drive, uint8_t *buf, uint32_t lba, uint8_t numsects);
status_t ata_dma_read(uint8_t drive, char *buf, uint32_t location, uint32_t length);
status_t ata_dma_write(uint8_t drive, const char *buf, uint32_t location, uint32_t length);