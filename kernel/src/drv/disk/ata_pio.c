//
// Created by maractus on 03.01.24.
//

#include "drv/disk/ata.h"
#include "drv/disk/ata_pio.h"
#include "io/ports.h"

extern ata_drive_t drives[4];

uint8_t ata_pio_read_sector(uint8_t drive, uint8_t *buf, uint32_t lba) {
    ON_NULLPTR(buf, {
            qemu_log("Buffer is nullptr!");
            return 0;
    });

    // Only 28-bit LBA supported!
    lba &= 0x00FFFFFF;

    uint16_t io = 0;

    if(!drives[drive].online) {
        qemu_log("Attempted read from drive that does not exist.");
        return 0;
    }

    ata_set_params(drive, &io, &drive);

    uint8_t cmd = (drive==ATA_MASTER?0xE0:0xF0);
    uint8_t slavebit = (drive == ATA_MASTER?0x00:0x01);

    outb(io + ATA_REG_HDDEVSEL, (cmd | (slavebit << 4) | (uint8_t)(lba >> 24 & 0x0F)));
    outb(io + 1, 0x00);
    outb(io + ATA_REG_SECCOUNT0, 1);
    outb(io + ATA_REG_LBA0, (uint8_t)((lba)));
    outb(io + ATA_REG_LBA1, (uint8_t)((lba) >> 8));
    outb(io + ATA_REG_LBA2, (uint8_t)((lba) >> 16));
    outb(io + ATA_REG_COMMAND, ATA_CMD_READ_PIO);

    ide_poll(io);

    uint16_t ata_data_reg = io + ATA_REG_DATA;

    uint16_t* buf16 = (uint16_t*)buf;

    for(int i = 0; i < 256; i++) {
        uint16_t data = inw(ata_data_reg);
        *(buf16 + i) = data;
    }

    ide_400ns_delay(io);

    return 1;
}

uint8_t ata_pio_read_sectors_pre(uint8_t drive, uint8_t *buf, uint32_t lba, uint8_t sector_count) {
    ON_NULLPTR(buf, {
            qemu_log("Buffer is nullptr!");
            return 0;
    });

    // Only 28-bit LBA supported!
    lba &= 0x00FFFFFF;

    uint16_t io = 0;

    if(!drives[drive].online) {
        qemu_log("Attempted read from drive that does not exist.");
        return 0;
    }

    ata_set_params(drive, &io, &drive);

    uint8_t cmd = (drive==ATA_MASTER?0xE0:0xF0);
    uint8_t slavebit = (drive == ATA_MASTER?0x00:0x01);

    outb(io + ATA_REG_HDDEVSEL, (cmd | (slavebit << 4) | (uint8_t)(lba >> 24 & 0x0F)));
    outb(io + 1, 0x00);
    outb(io + ATA_REG_SECCOUNT0, sector_count);
    outb(io + ATA_REG_LBA0, (uint8_t)((lba)));
    outb(io + ATA_REG_LBA1, (uint8_t)((lba) >> 8));
    outb(io + ATA_REG_LBA2, (uint8_t)((lba) >> 16));
    outb(io + ATA_REG_COMMAND, ATA_CMD_READ_PIO);

    ide_poll(io);

    uint16_t ata_data_reg = io + ATA_REG_DATA;

    uint16_t* buf16 = (uint16_t*)buf;

    for(int i = 0; i < 256 * sector_count; i++) {
        uint16_t data = inw(ata_data_reg);
        *(buf16 + i) = data;
    }

    ide_400ns_delay(io);

    return 1;
}

/**
 * @brief Полностью перезаписывает сектор на диске
 * @param drive Номер диска
 * @param buf Юуффер с данными
 * @param lba Номер сектора
 * @return 0 - ошибка, 1 - ок
 */
uint8_t ata_pio_write_raw_sector(uint8_t drive, const uint8_t *buf, uint32_t lba) {
    ON_NULLPTR(buf, {
            qemu_log("Buffer is nullptr!");
            return 0;
    });

    // Only 28-bit LBA supported!
    lba &= 0x00FFFFFF;

    uint16_t io = 0;

    if(!drives[drive].online) {
        qemu_log("Attempted read from drive that does not exist.");
        return 0;
    }

    ata_set_params(drive, &io, &drive);

    uint8_t cmd = (drive==ATA_MASTER?0xE0:0xF0);
    uint8_t slavebit = (drive == ATA_MASTER?0x00:0x01);

    outb(io + ATA_REG_HDDEVSEL, (cmd | (slavebit << 4) | (uint8_t)((lba >> 24 & 0x0F))));
    outb(io + 1, 0x00);
    outb(io + ATA_REG_SECCOUNT0, 1);
    outb(io + ATA_REG_LBA0, (uint8_t)((lba)));
    outb(io + ATA_REG_LBA1, (uint8_t)((lba) >> 8));
    outb(io + ATA_REG_LBA2, (uint8_t)((lba) >> 16));
    outb(io + ATA_REG_COMMAND, ATA_CMD_WRITE_PIO);

    ide_poll(io);

    for(int i = 0; i < 256; i++) {
        outw(io + ATA_REG_DATA, *(uint16_t*)(buf + i * 2));
        ide_400ns_delay(io);
    }

    outb(io + ATA_REG_COMMAND, ATA_CMD_CACHE_FLUSH);

    return 1;
}

// UNTESTED
void ata_pio_write_sectors(uint8_t drive, uint8_t *buf, uint32_t lba, size_t sectors) {
    ON_NULLPTR(buf, {
            qemu_log("Buffer is nullptr!");
            return;
    });

    for(size_t i = 0; i < sectors; i++) {
        ata_pio_write_raw_sector(drive, buf + (i * drives[drive].block_size), lba + i);
    }
}

// Optimize here - copy content of ata_pio_read_sector and adjust number of sectors
// Or modify function to accept `n` sectors
void ata_pio_read_sectors(uint8_t drive, uint8_t *buf, uint32_t lba, uint32_t numsects) {
    ON_NULLPTR(buf, {
            qemu_log("Buffer is nullptr!");
            return;
    });

    uint8_t* rbuf = buf;

    for(size_t i = 0; i < numsects; i++) {
        ata_pio_read_sector(drive, rbuf, lba + i);
        rbuf += drives[drive].block_size;
    }
}

