#pragma once

#include "common.h"
#include "drv/ata.h"
#include "io/ports.h"

#define ATAPI_CMD_READY      0x00
#define ATAPI_CMD_RQ_SENSE   0x03
#define ATAPI_CMD_START_STOP 0x1B
#define ATAPI_READ_CAPACITY  0x25
#define ATAPI_CMD_READ       0xA8

typedef struct {
    bool valid;
    
    uint8_t sense_key;
    uint8_t sense_code;
    uint8_t sense_code_qualifier;
} atapi_error_code;

bool ata_scsi_status_wait(uint8_t bus);
bool ata_scsi_send(uint16_t bus, bool slave, uint16_t lba_mid_hi, uint8_t command[12]);

size_t ata_scsi_receive_size_of_transfer(uint16_t bus);
void ata_scsi_read_result(uint16_t bus, size_t size, uint16_t* buffer);
size_t atapi_read_size(uint16_t bus, bool slave);
size_t atapi_read_block_size(uint16_t bus, bool slave);

bool atapi_read_sectors(uint16_t drive, uint8_t *buf, uint32_t lba, size_t sector_count);
bool atapi_eject(uint8_t bus, bool slave);

/// Returns true if present, false otherwise
bool atapi_check_media_presence(uint8_t bus, bool slave);
atapi_error_code atapi_request_sense(uint8_t bus, bool slave, uint8_t out[18]);