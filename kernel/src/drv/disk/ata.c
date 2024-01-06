#include "drv/disk/ata.h"
#include <io/ports.h>
#include <drv/atapi.h>
#include <drv/disk/ndpm.h>
#include <fs/fsm.h>
#include <mem/vmm.h>
#include <io/tty.h>
#include "drv/disk/dpm.h"
#include "sys/isr.h"
#include "debug/hexview.h"
#include "net/endianess.h"

#define DEFAULT_TIMEOUT (65535 * 2)

const char possible_dpm_letters_for_ata[4] = "CDEF";

// PRIMARY MASTER
// PRIMARY SLAVE
// SECONDARY MASTER
// SECONDARY SLAVE
ata_drive_t drives[4] = {0};

uint16_t *ide_buf = 0;

extern uint16_t ata_dma_bar4;

void ide_select_drive(uint8_t bus, bool slave) {
	if(bus == ATA_PRIMARY)
		outb(ATA_PRIMARY_IO + ATA_REG_HDDEVSEL, (0xA0 | ((uint8_t)slave << 4)));
	else
		outb(ATA_SECONDARY_IO + ATA_REG_HDDEVSEL, (0xA0 | ((uint8_t)slave << 4)));
}

void ide_primary_irq(__attribute__((unused)) registers_t regs) {
//	qemu_log("=================== Got ATA interrupt. PRIMARY");

	uint8_t status = inb(ATA_PRIMARY_IO + ATA_REG_STATUS);

//	qemu_log("Status: %d (%x); Altstatus: %x", status, status, altstatus);
}

void ide_secondary_irq(__attribute__((unused)) registers_t regs) {
//	qemu_log("=================== Got ATA interrupt. SECONDARY");

	size_t status = inb(ATA_SECONDARY_IO + ATA_REG_STATUS);

//	qemu_log("Status: %d (%x); Altstatus: %x", status, status, altstatus);
}

void ide_soft_reset(size_t io) {
	outb(io + ATA_REG_CONTROL, 0x04);
	ide_400ns_delay(io);
	outb(io + ATA_REG_CONTROL, 0);
}

size_t dpm_ata_pio_read(size_t Disk, size_t Offset, size_t Size, void* Buffer){
    /// Функции для чтения
    DPM_Disk dpm = dpm_info(Disk + 65);
    qemu_note("[ATA] [DPM] [DISK %d] [READ] Off: %d | Size: %d", dpm.Point, Offset, Size);
    // TODO: @ndraey не забудь для своей функции сделать кол-во полученных байт
	// FIXME: For those who want to see thid piece of code: I literally burned my eyes with this.
    ata_pio_read((uint8_t)dpm.Point, Buffer, Offset, Size);
    return Size;
}


size_t dpm_ata_pio_write(size_t Disk, size_t Offset, size_t Size, void* Buffer){
    /// Функции для записи
    DPM_Disk dpm = dpm_info(Disk + 65);
    qemu_note("[ATA] [DPM] [DISK %d] [WRITE] Off: %d | Size: %d", dpm.Point, Offset, Size);
    // TODO: @ndraey не забудь для своей функции сделать кол-во записанных байт
	// FIXME: For those who want to see thid piece of code: I literally burned my eyes with this.
    ata_pio_write((uint8_t)dpm.Point, Buffer, Offset, Size);
    return Size;
}


uint8_t ide_identify(uint8_t bus, uint8_t drive) {
	uint16_t io = (bus == ATA_PRIMARY) ? ATA_PRIMARY_IO : ATA_SECONDARY_IO;

	uint8_t drive_num = (bus << 1) | drive;

	qemu_log("Identifying %s %s", PRIM_SEC(bus), MAST_SLV(drive));

	ide_soft_reset(io);
	ide_select_drive(bus, drive);

	outb(io + ATA_REG_SECCOUNT0, 0);
	outb(io + ATA_REG_LBA0, 0);
	outb(io + ATA_REG_LBA1, 0);
	outb(io + ATA_REG_LBA2, 0);
	
	outb(io + ATA_REG_COMMAND, ATA_CMD_IDENTIFY);
	qemu_log("Sent IDENTIFY");
	
	uint8_t status = inb(io + ATA_REG_STATUS);

	qemu_log("Status: %d; Err: %d", status, status & ATA_SR_ERR);

	size_t timeout = DEFAULT_TIMEOUT;

	if(status) {
		// In ATAPI, IDENTIFY command has ERROR bit set.
		
		uint8_t seccount = inb(io + ATA_REG_SECCOUNT0);
		uint8_t lba_l = inb(io + ATA_REG_LBA0);
		uint8_t lba_m = inb(io + ATA_REG_LBA1);
		uint8_t lba_h = inb(io + ATA_REG_LBA2);

		qemu_warn("%x %x %x %x", seccount, lba_l, lba_m, lba_h);
		// ^----- If they contain 0x01, 0x01, 0x14, 0xEB then the device is a packet device,
		// and `IDENTIFY PACKET DEVICE` (0xA1) should be used. 

		if(seccount == 0x01 && lba_l == 0x01 && lba_m == 0x14 && lba_h == 0xEB) {
			// It's ATAPI device! Yoo-hooo!
			
			qemu_log("ATA Packet Device!");
	
			drives[drive_num].online = true;
			drives[drive_num].is_packet = true;
		
			outb(io + ATA_REG_COMMAND, ATA_CMD_IDENTIFY_PACKET);
			 
			 for(int i = 0; i < 256; i++) {
			 	*(uint16_t *)(ide_buf + i) = inw(io + ATA_REG_DATA);
			 }

			uint16_t* fwver = kcalloc(8, 1);
			uint16_t* model_name = kcalloc(40, 1);
			uint16_t* serial = kcalloc(20, 1);

			memcpy(serial, ide_buf + 10, 20);
			memcpy(fwver, ide_buf + 23, 8);
			memcpy(model_name, ide_buf + 27, 40);

			for(int i = 0; i < 10; i++) {
				serial[i] = bit_flip_short(serial[i]);
			}

			for(int i = 0; i < 4; i++) {
				fwver[i] = bit_flip_short(fwver[i]);
			}

			for(int i = 0; i < 20; i++) {
				model_name[i] = bit_flip_short(model_name[i]);
			}

			// Zero-terminate the strings
			((uint8_t*)serial)[19] = 0;
			((uint8_t*)fwver)[7] = 0;
			((uint8_t*)model_name)[39] = 0;


			// Do my best for processing packet device.

			drives[drive_num].capacity = atapi_read_size(bus, drive);
			drives[drive_num].block_size = atapi_read_block_size(bus, drive);

            drives[drive_num].fwversion = (char *) fwver;
            drives[drive_num].model_name = (char *) model_name;
            drives[drive_num].serial_number = (char *) serial;

            qemu_log("Size is: %d", drives[drive_num].capacity);

			memset(ide_buf, 0, 512);

            qemu_note("DRIVE: %d", drive_num);

            qemu_note("Serial: %s", serial);
            qemu_note("Firmware version: %s", fwver);
            qemu_note("Model name: %s", model_name);

			ndpm_add_drive(&(drives[drive_num]), ata_ndpm_read, ata_ndpm_write);

            // (drive_num) is an index (0, 1, 2, 3) of disk
            int disk_inx = dpm_reg(
                    possible_dpm_letters_for_ata[drive_num],
                    "CD\\DVD drive",
                    "Unknown",
                    1,
                    drives[drive_num].capacity * drives[drive_num].block_size,
                    drives[drive_num].capacity,
                    drives[drive_num].block_size,
                    3, // Ставим 3ку, так как будем юзать функции для чтения и записи
                    "DISK1234567890",
                    (void*)drive_num // Оставим тут индекс диска
            );

            if (disk_inx < 0){
                qemu_err("[ATA] [DPM] [ERROR] An error occurred during disk registration, error code: %d",disk_inx);

            } else {
                qemu_ok("[ATA] [DPM] [Successful] [is_packet: %d] Your disk index: %d",drives[drive_num].is_packet, disk_inx);
                dpm_fnc_write(disk_inx + 65, &dpm_ata_pio_read, &dpm_ata_pio_write);
            }

			return 0;
		} else if(seccount == 0x7F && lba_l == 0x7F && lba_m == 0x7F && lba_h == 0x7F) {
			qemu_err("Error possible (Virtualbox returns 0x7f 0x7f 0x7f 0x7f for non-existent drives)");
			return 1;
		}

		/* Now, poll until BSY is clear. */
		while((status & ATA_SR_BSY) != 0){
			qemu_log("Got status %x", status);
			if(status & ATA_SR_ERR) {
				qemu_log("%s %s has ERR set. Disabled.", PRIM_SEC(bus), MAST_SLV(drive));
				return 1;
			}

			if(!timeout) {
				qemu_log("ATA Timeout expired!");
				return 1;
			} else {
				timeout--;
			}

			status = inb(io + ATA_REG_STATUS);
		}

		timeout = DEFAULT_TIMEOUT;

		while(!(status & ATA_SR_DRQ)) {
			qemu_log("Got status %x", status);
			if(status & ATA_SR_ERR) {
				qemu_log("%s %s has ERR set. Disabled.", PRIM_SEC(bus), MAST_SLV(drive));
				return 1;
			}

			if(!timeout) {
				qemu_log("ATA Timeout expired!");
				return 1;
			}

			timeout--;

			status = inb(io + ATA_REG_STATUS);
		}
		
		drives[drive_num].online = true;

		qemu_log("%s %s is online.", PRIM_SEC(bus), MAST_SLV(drive));

		for(int i = 0; i < 256; i++) {
			*(uint16_t *)(ide_buf + i) = inw(io + ATA_REG_DATA);
		}

		uint16_t capacity_lba = ide_buf[ATA_IDENT_MAX_LBA];
		uint16_t capacity_lba_ext = ide_buf[ATA_IDENT_MAX_LBA_EXT];

		uint16_t cyl = ide_buf[ATA_IDENT_CYLINDERS];
		uint16_t hds = ide_buf[ATA_IDENT_HEADS];
		uint16_t scs = ide_buf[ATA_IDENT_SECTORS];

		// Dump model and firmware version
		drives[drive_num].fwversion = kcalloc(8, 1);
		drives[drive_num].model_name = kcalloc(40, 1);
		drives[drive_num].serial_number = kcalloc(20, 1);

		uint16_t* fwver = (uint16_t *) drives[drive_num].fwversion;
		uint16_t* model_name = (uint16_t *) drives[drive_num].model_name;
		uint16_t* serial = (uint16_t *) drives[drive_num].serial_number;

		memcpy(serial, ide_buf + 10, 20);
		memcpy(fwver, ide_buf + 23, 8);
		memcpy(model_name, ide_buf + 27, 40);

		for(int i = 0; i < 10; i++) {
			serial[i] = bit_flip_short(serial[i]);
		}

		for(int i = 0; i < 4; i++) {
			fwver[i] = bit_flip_short(fwver[i]);
		}

		for(int i = 0; i < 20; i++) {
			model_name[i] = bit_flip_short(model_name[i]);
		}

		// Zero-terminate the strings
		((uint8_t*)serial)[19] = 0;
		((uint8_t*)fwver)[7] = 0;
		((uint8_t*)model_name)[39] = 0;

		size_t capacity = capacity_lba;

		if(!capacity_lba) {
			if(!capacity_lba_ext) {
				// capacity = (cyl * hds * scs) + 128;
				drives[drive_num].is_chs_addressing = true;

				drives[drive_num].cylinders = cyl;
				drives[drive_num].heads = hds;
				drives[drive_num].sectors = scs;
			} else {
				capacity = capacity_lba_ext;
			}
		}

		drives[drive_num].drive = drive_num;
		drives[drive_num].block_size = 512;
		drives[drive_num].capacity = capacity;

		// (drive_num) is an index (0, 1, 2, 3) of disk
		int disk_inx = dpm_reg(
				possible_dpm_letters_for_ata[drive_num],
				"ATA IDE Disk",
				"Unknown",
				1,
				capacity * 512,
				capacity,
				drives[drive_num].block_size,
				3, // Ставим 3ку, так как будем юзать функции для чтения и записи
				"DISK1234567890",
                (void*)drive_num // Оставим тут индекс диска
		);

        if (disk_inx < 0){
            qemu_err("[ATA] [DPM] [ERROR] An error occurred during disk registration, error code: %d",disk_inx);
        } else {
            qemu_ok("[ATA] [DPM] [Successful] [is_packet: %d] Your disk index: %d",drives[drive_num].is_packet, disk_inx);
            dpm_fnc_write(disk_inx + 65, &dpm_ata_pio_read, &dpm_ata_pio_write);
        }

		ndpm_add_drive(drives + drive_num, ata_ndpm_read, ata_ndpm_write);

		qemu_log("Identify finished");
	}else{
		qemu_err("%s %s => No status. Drive may be disconnected!", PRIM_SEC(bus), MAST_SLV(drive));
		return 1;
	}

	return 0;
}

void ide_400ns_delay(uint16_t io) {
	inb(io + ATA_REG_ALTSTATUS);
	inb(io + ATA_REG_ALTSTATUS);
	inb(io + ATA_REG_ALTSTATUS);
	inb(io + ATA_REG_ALTSTATUS);
}

// void ide_poll(uint16_t io) {
// 	ide_400ns_delay(io);

// retry:;
// 	uint8_t status = inb(io + ATA_REG_STATUS);

// 	if(status & ATA_SR_BSY)
// 		goto retry;
// retry2:	status = inb(io + ATA_REG_STATUS);
// 	if(status & ATA_SR_ERR)
// 	{
// 		qemu_log("ERR set, device failure!\n");
// 	}
	
// 	if(!(status & ATA_SR_DRQ))
// 		goto retry2;
	
// 	return;
// }

void ide_poll(uint16_t io) {
	ide_400ns_delay(io);

	uint8_t status __attribute__((unused)) = inb(io + ATA_REG_STATUS);
	
	while(1) {
		status = inb(io + ATA_REG_STATUS);

		if(!(status & ATA_SR_BSY))
			break;
	}

	while(1) {
		status = inb(io + ATA_REG_STATUS);
		if(status & ATA_SR_ERR) {
            qemu_err("ERR set, device failure!\n");
            break;
        }
		
		if(status & ATA_SR_DRQ)
			break;
	}
}

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

// UNTESTED
void ata_pio_read(uint8_t drive, uint8_t* buf, uint32_t location, uint32_t length) {
	ON_NULLPTR(buf, {
		qemu_log("Buffer is nullptr!");
		return;
	});

	if(!drives[drive].online) {
		qemu_log("Attempted read from drive that does not exist.");
		return;
	}
	
	size_t start_sector = location / drives[drive].block_size;
	size_t end_sector = (location + length - 1) / drives[drive].block_size;
	size_t sector_count = end_sector - start_sector + 1;
	
	size_t real_length = sector_count * drives[drive].block_size;

	qemu_log("Reading %d sectors...", sector_count);

	uint8_t* real_buf = kmalloc(real_length);

	if(!drives[drive].is_packet) {
		ata_pio_read_sectors(drive, real_buf, start_sector, sector_count);
	} else {
		atapi_read_sectors(drive, real_buf, start_sector, sector_count);
	}

	memcpy(buf, real_buf + (location % drives[drive].block_size), length);

	kfree(real_buf);
}

void ata_pio_write(uint8_t drive, const uint8_t* buf, size_t location, size_t length) {
	ON_NULLPTR(buf, {
		qemu_log("Buffer is nullptr!");
		return;
	});

	if(!drives[drive].online) {
		qemu_log("Attempted read from drive that does not exist.");
		return;
	}
	
	size_t start_sector = location / drives[drive].block_size;
    size_t end_sector = (location + length - 1) / drives[drive].block_size;
    size_t sector_count = end_sector - start_sector + 1;
    
    uint8_t* temp_buf = kmalloc(sector_count * drives[drive].block_size);

	ata_pio_read_sectors(drive, temp_buf, start_sector, sector_count);
    
    size_t start_offset = location % drives[drive].block_size;
    memcpy(temp_buf + start_offset, buf, length);

	ata_pio_write_sectors(drive, temp_buf, start_sector, sector_count);
    
    kfree(temp_buf);
}

void ata_ndpm_read(const ndpm_drive_t* drive, size_t location, int size, void* buffer) {
	ata_drive_t* disk = drive->drive_specific_data;

	qemu_note("Reading disk: %d", disk->drive);

	ata_pio_read(disk->drive, buffer, location, size);
}

void ata_ndpm_write(const ndpm_drive_t* drive, size_t location, int size, void* buffer) {
	ata_drive_t* disk = drive->drive_specific_data;

	qemu_note("Writing disk: %d", disk->drive);

	ata_pio_write(disk->drive, buffer, location, size);
}

void ata_list() {
	for (size_t i = 0; i < 4; i++) {
		_tty_printf("\tATA: %s %s:  %s ",
					PRIM_SEC((i >> 1) & 1),
					MAST_SLV(i & 1),
					drives[i].online?"online ":"offline"
		);

		if(!drives[i].online) {
			_tty_printf("\n");
			continue;
		}

		if(!drives[i].is_chs_addressing) {
			_tty_printf("(%d bytes | %d KB | %d MB)",
					(drives[i].capacity * drives[i].block_size),
					(drives[i].capacity * drives[i].block_size) >> 10,
					(drives[i].capacity * drives[i].block_size) >> 20);
		} else {
			_tty_printf("(C:H:S => %d:%d:%d)",
					drives[i].cylinders,
					drives[i].heads,
					drives[i].sectors);
		}

		if(drives[i].is_packet)
			_tty_printf(" [PACKET DEVICE!!!]");
		
		if(drives[i].is_sata)
			_tty_printf(" [SATA]");

		qemu_note("Drive %d", i);
		qemu_note("'%s' '%s' '%s'", drives[i].model_name, drives[i].fwversion, drives[i].serial_number);

		_tty_printf("\n\t|-- Model: \"%s\"; Firmware version: \"%s\";", drives[i].model_name, drives[i].fwversion);
		_tty_printf("\n\t|-- Serial number: \"%s\";", drives[i].serial_number);

		_tty_printf("\n");
	}
	
	tty_printf("\n");
}

ata_drive_t* ata_get_drives() {
	return drives;
}

void ata_check_all() {
	ide_identify(ATA_PRIMARY, ATA_MASTER);
	ide_identify(ATA_PRIMARY, ATA_SLAVE);
	ide_identify(ATA_SECONDARY, ATA_MASTER);
	ide_identify(ATA_SECONDARY, ATA_SLAVE);
}

void ata_init() {
	qemu_log("Checking for ATA drives");

	ide_buf = (uint16_t*)kcalloc(512, 1);
	
	register_interrupt_handler(32 + ATA_PRIMARY_IRQ, ide_primary_irq); // Fuck IRQs
	register_interrupt_handler(32 + ATA_SECONDARY_IRQ, ide_secondary_irq);

	ata_check_all();

	kfree(ide_buf);
}
