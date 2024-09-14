#pragma once

#include "../../common.h"

#define ATA_SR_BSY     0x80
#define ATA_SR_DRDY    0x40
#define ATA_SR_DF      0x20
#define ATA_SR_DSC     0x10
#define ATA_SR_DRQ     0x08
#define ATA_SR_CORR    0x04
#define ATA_SR_IDX     0x02
#define ATA_SR_ERR     0x01

#define ATA_ER_BBK      0x80
#define ATA_ER_UNC      0x40
#define ATA_ER_MC       0x20
#define ATA_ER_IDNF     0x10
#define ATA_ER_MCR      0x08
#define ATA_ER_ABRT     0x04
#define ATA_ER_TK0NF    0x02
#define ATA_ER_AMNF     0x01

#define ATA_CMD_READ_PIO          0x20
#define ATA_CMD_READ_PIO_EXT      0x24
#define ATA_CMD_READ_DMA          0xC8
#define ATA_CMD_READ_DMA_EXT      0x25
#define ATA_CMD_WRITE_PIO         0x30
#define ATA_CMD_WRITE_PIO_EXT     0x34
#define ATA_CMD_WRITE_DMA         0xCA
#define ATA_CMD_WRITE_DMA_EXT     0x35
#define ATA_CMD_CACHE_FLUSH       0xE7
#define ATA_CMD_CACHE_FLUSH_EXT   0xEA
#define ATA_CMD_PACKET            0xA0
#define ATA_CMD_IDENTIFY_PACKET   0xA1
#define ATA_CMD_IDENTIFY          0xEC

#define ATA_IDENT_DEVICETYPE   0
#define ATA_IDENT_CYLINDERS    1
#define ATA_IDENT_HEADS        3
#define ATA_IDENT_SECTORS      6
#define ATA_IDENT_SERIAL       10
#define ATA_IDENT_MODEL        27
#define ATA_IDENT_CAPABILITIES 49
#define ATA_IDENT_FIELDVALID   53
#define ATA_IDENT_MAX_LBA      60
#define ATA_IDENT_COMMANDSETS  82
#define ATA_IDENT_MAX_LBA_EXT  100

#define ATA_MASTER     0x00
#define ATA_SLAVE      0x01

#define ATA_DMA_PRIMARY_CMD 0x00
#define ATA_DMA_PRIMARY_STATUS 0x02
#define ATA_DMA_PRIMARY_PRDT 0x04

#define ATA_DMA_SECONDARY_CMD 0x08
#define ATA_DMA_SECONDARY_STATUS 0x0A
#define ATA_DMA_SECONDARY_PRDT 0x0C

#define ATA_REG_DATA       0x00
#define ATA_REG_ERROR      0x01
#define ATA_REG_FEATURES   0x01
#define ATA_REG_SECCOUNT0  0x02
#define ATA_REG_LBA0       0x03
#define ATA_REG_LBA1       0x04
#define ATA_REG_LBA2       0x05
#define ATA_REG_HDDEVSEL   0x06
#define ATA_REG_COMMAND    0x07
#define ATA_REG_STATUS     0x07
#define ATA_REG_SECCOUNT1  0x08
#define ATA_REG_LBA3       0x09
#define ATA_REG_LBA4       0x0A
#define ATA_REG_LBA5       0x0B
#define ATA_REG_CONTROL    0x0C
#define ATA_REG_ALTSTATUS  0x0C
#define ATA_REG_DEVADDRESS 0x0D

// Channels:
#define      ATA_PRIMARY      0x00
#define      ATA_SECONDARY    0x01
 
// Directions:
#define      ATA_READ      0x00
#define      ATA_WRITE     0x013


#define ATA_PRIMARY_IO 0x1F0
#define ATA_SECONDARY_IO 0x170

#define ATA_PRIMARY_DCR_AS 0x3F6
#define ATA_SECONDARY_DCR_AS 0x376

#define ATA_PRIMARY_IRQ 14
#define ATA_SECONDARY_IRQ 15

#define PRIM_SEC(bus) ((bus) == ATA_PRIMARY?"Primary  ":"Secondary")
#define MAST_SLV(drive) ((drive) == ATA_MASTER?"master":"slave ")
#define DRIVE(bus, drive) ((bus) << 1 | (drive))
#define ATA_PORT(bus) ((bus) == ATA_PRIMARY ? ATA_PRIMARY_IO : ATA_SECONDARY_IO)

/// Эта структура определяет каждый ATA диск в системе
typedef struct {
	uint8_t drive;
    bool online;            /// Устройство доступно?
    size_t capacity;        /// Ёмкость диска в секторах

    bool is_packet;         /// Является ли пакетным устройством?
	bool is_medium_inserted;
    bool is_chs_addressing; /// Адресация по CHS?

    uint16_t block_size;    /// Размер сектора

    uint16_t cylinders;     /// Цилиндры
    uint16_t heads;         /// Головки
    uint16_t sectors;       /// Сектора

    bool is_sata;           /// Является ли этот диск SATA устройством?
    bool is_dma;           /// Поддерживает ли этот диск DMA?

	char* model_name;
	char* fwversion;
	char* serial_number;
} ata_drive_t;


void ide_select_drive(uint8_t bus, bool slave);
void ide_400ns_delay(uint16_t io);
void ide_poll(uint16_t io);
bool ide_poll_drq(uint16_t io);
bool ide_poll_bsy(uint16_t io);

void ata_read(uint8_t drive, uint8_t* buf, uint32_t location, uint32_t length) ;
void ata_write(uint8_t drive, const uint8_t* buf, size_t location, size_t length);

void ata_list();
void ata_init();

ata_drive_t* ata_get_drives();
void ata_check_all();

uint8_t ide_identify(uint8_t bus, uint8_t drive);

static inline void ata_set_params(uint8_t drive, uint16_t* io, uint8_t* real_drive) {
	uint8_t _io = drive >> 1; // PRIMARY / SECONDARY
	uint8_t _drv = drive & 1; // MASTER / SLAVE

	if(_io == ATA_PRIMARY)
		*io = ATA_PRIMARY_IO;
	else if(_io == ATA_SECONDARY)
		*io = ATA_SECONDARY_IO;

	*real_drive = _drv;
}
