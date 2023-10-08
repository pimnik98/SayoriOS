#include <drv/ata.h>
#include <io/ports.h>
#include <drv/atapi.h>
#include <sys/memory.h>
#include <io/tty.h>

#define DEFAULT_TIMEOUT (65535 * 2)

// PRIMARY MASTER
// PRIMARY SLAVE
// SECONDARY MASTER
// SECONDARY SLAVE
ata_drive_t drives[4] = {0};

uint16_t *ide_buf = 0;

extern uint16_t ata_dma_bar4;

void ide_select_drive(uint8_t bus, bool slave) {
	if(bus == ATA_PRIMARY)
		outb(ATA_PRIMARY_IO + ATA_REG_HDDEVSEL, (0xA0 | (slave << 4)));
	else
		outb(ATA_SECONDARY_IO + ATA_REG_HDDEVSEL, (0xA0 | (slave << 4)));
}

void ide_primary_irq(__attribute__((unused)) registers_t regs) {
	qemu_log("=================== Got ATA interrupt. PRIMARY");

	uint8_t status = inb(ATA_PRIMARY_IO + ATA_REG_STATUS);

	qemu_log("Status: %d (%x)", status, status);
}

void ide_secondary_irq(__attribute__((unused)) registers_t regs) {
	qemu_log("=================== Got ATA interrupt. SECONDARY");

	size_t status = inb(ATA_SECONDARY_IO + ATA_REG_STATUS);

	qemu_log("Status: %d (%x)", status, status);
}

uint8_t ide_identify(uint8_t bus, uint8_t drive) {
	uint16_t io;

	qemu_log("Identifying %s %s", PRIM_SEC(bus), MAST_SLV(drive));
	
	ide_select_drive(bus, drive);

	if(bus == ATA_PRIMARY)
		io = ATA_PRIMARY_IO;
	else
		io = ATA_SECONDARY_IO;
	
	outb(io + ATA_REG_SECCOUNT0, 0);
	outb(io + ATA_REG_LBA0, 0);
	outb(io + ATA_REG_LBA1, 0);
	outb(io + ATA_REG_LBA2, 0);
	
	outb(io + ATA_REG_COMMAND, ATA_CMD_IDENTIFY);
	qemu_log("Sent IDENTIFY");
	
	uint8_t status = inb(io + ATA_REG_STATUS);

	size_t timeout = DEFAULT_TIMEOUT;

	if(status) {
		// In ATAPI, IDENTIFY command has ERROR bit set.
		
		uint8_t seccount = inb(io + ATA_REG_SECCOUNT0);
		uint8_t lba_l = inb(io + ATA_REG_LBA0);
		uint8_t lba_m = inb(io + ATA_REG_LBA1);
		uint8_t lba_h = inb(io + ATA_REG_LBA2);

		qemu_log("%x %x %x %x", seccount, lba_l, lba_m, lba_h);

		// ^----- If they contain 0x01, 0x01, 0x14, 0xEB then the device is a packet device,
		// and `IDENTIFY PACKET DEVICE` (0xA1) should be used. 

		if(seccount == 0x01 && lba_l == 0x01 && lba_m == 0x14 && lba_h == 0xEB) {
			// It's ATAPI device! Yoo-hooo!
			
			qemu_log("ATA Packet Device!");
	
			drives[(bus << 1) | drive].online = true;
			drives[(bus << 1) | drive].is_packet = true;
		
			// outb(io + ATA_REG_COMMAND, ATA_CMD_IDENTIFY_PACKET);

			// for(int i = 0; i < 256; i++) {
			// 	*(uint16_t *)(ide_buf + i * 2) = ins(io + ATA_REG_DATA);
			// }

			// Do my best for processing packet device.

			// 1. Get disc content size

			drives[(bus << 1) | drive].capacity = atapi_read_size(bus, drive);
			drives[(bus << 1) | drive].block_size = atapi_read_block_size(bus, drive);

			qemu_log("Size is: %d", drives[(bus << 1) | drive].capacity);

			return 0;
		} else if(lba_m == 0x3C && lba_h == 0xC3) {
			// https://wiki.osdev.org/SATA

			// Send a standard IDENTIFY command to the drive (0xEC).
			// The drive should respond with an error in the ERR bit of the Status Register,
			// and a pair of "signature bytes".
			
			// On the Primary ATA bus, you get the signature bytes by reading IO ports 0x1F4 and 0x1F5,
			// and you should see values of 0x3C and 0xC3. 

			// NDRAEY: What about Secondary bus? 0x174 and 0x175?

			qemu_log("FUCKING SATA!!!");

			drives[DRIVE(bus, drive)].is_sata = true;

			return 0;
		}

		/* Now, poll until BSY is clear. */
		while((inb(io + ATA_REG_STATUS) & ATA_SR_BSY) != 0){
			status = inb(io + ATA_REG_STATUS);
		
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
		}

		timeout = DEFAULT_TIMEOUT;

		while(!(status & ATA_SR_DRQ)) {
			status = inb(io + ATA_REG_STATUS);
		
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
		}
		
		drives[(bus << 1) | drive].online = true;

		qemu_log("%s %s is online.", PRIM_SEC(bus), MAST_SLV(drive));

		for(int i = 0; i < 256; i++) {
			*(uint16_t *)(ide_buf + i * 2) = ins(io + ATA_REG_DATA);
		}
		
		uint16_t capacity_lba = ide_buf[ATA_IDENT_MAX_LBA];
		uint16_t capacity_lba_ext = ide_buf[ATA_IDENT_MAX_LBA_EXT];

		uint16_t cyl = ide_buf[ATA_IDENT_CYLINDERS];
		uint16_t hds = ide_buf[ATA_IDENT_HEADS];
		uint16_t scs = ide_buf[ATA_IDENT_SECTORS];

		size_t capacity = capacity_lba;

		if(!capacity_lba) {
			if(!capacity_lba_ext) {
				// capacity = (cyl * hds * scs) + 128;
				drives[(bus << 1) | drive].is_chs_addressing = true;

				drives[(bus << 1) | drive].cylinders = cyl;
				drives[(bus << 1) | drive].heads = hds;
				drives[(bus << 1) | drive].sectors = scs;
			} else {
				capacity = capacity_lba_ext;
			}
		}

		drives[(bus << 1) | drive].block_size = 512;
		drives[(bus << 1) | drive].capacity = capacity * 512;

		qemu_log("Identify finished");
	}else{
		qemu_log("%s %s => No status. Drive may be disconnected!", PRIM_SEC(bus), MAST_SLV(drive));
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
		if(status & ATA_SR_ERR)
			qemu_log("ERR set, device failure!\n");
			// break;
		
		if(status & ATA_SR_DRQ)
			break;
	}
}

static inline void ata_set_params(uint8_t drive, uint16_t* io, uint8_t* real_drive) {
	uint8_t _io = drive >> 1;
	uint8_t _drv = drive & 1;

	if(_io == ATA_PRIMARY)
		*io = ATA_PRIMARY_IO;
	else if(_io == ATA_SECONDARY)
		*io = ATA_SECONDARY_IO;

	*real_drive = _drv;
}

uint8_t ata_read_sector(uint8_t drive, uint8_t *buf, uint32_t lba) {
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
		uint16_t data = ins(ata_data_reg);
		*(buf16 + i) = data;
	}

	ide_400ns_delay(io);
	
	return 1;
}

uint8_t ata_write_raw_sector(uint8_t drive, const uint8_t *buf, uint32_t lba) {
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
		outs(io + ATA_REG_DATA, *(uint16_t*)(buf + i * 2));
		ide_400ns_delay(io);
	}

	outb(io + ATA_REG_COMMAND, ATA_CMD_CACHE_FLUSH);
	
	return 1;
}

// UNTESTED
void ata_write_sectors(uint8_t drive, uint8_t *buf, uint32_t lba, size_t sectors) {
	ON_NULLPTR(buf, {
		qemu_log("Buffer is nullptr!");
		return;
	});

	for(size_t i = 0; i < sectors; i++) {
		ata_write_raw_sector(drive, buf + (i * drives[drive].block_size), lba + i);
	}
}

void ata_read_sectors(uint8_t drive, uint8_t *buf, uint32_t lba, uint32_t numsects) {
	ON_NULLPTR(buf, {
		qemu_log("Buffer is nullptr!");
		return;
	});

	uint8_t* rbuf = buf;
	
	for(size_t i = 0; i < numsects; i++) {
		ata_read_sector(drive, rbuf, lba + i);
		rbuf += drives[drive].block_size;
	}
}

// UNTESTED
void ata_read(uint8_t drive, uint8_t* buf, uint32_t location, uint32_t length) {
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
		ata_read_sectors(drive, real_buf, start_sector, sector_count);
	} else {
		atapi_read_sectors(drive, real_buf, start_sector, sector_count);
	}

	memcpy(buf, real_buf + (location % drives[drive].block_size), length);

	kfree(real_buf);
}

void ata_write(uint8_t drive, const uint8_t* buf, size_t location, size_t length) {
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
    
    ata_read_sectors(drive, temp_buf, start_sector, sector_count);
    
    size_t start_offset = location % drives[drive].block_size;
    memcpy(temp_buf + start_offset, buf, length);
    
    ata_write_sectors(drive, temp_buf, start_sector, sector_count);
    
    kfree(temp_buf);
}


void ata_list() {
	for (size_t i = 0; i < 4; i++) {
		_tty_printf("\tATA: %s %s:  %s ",
					PRIM_SEC((i >> 1) & 1),
					MAST_SLV(i & 1),
					drives[i].online?"online ":"offline"
		);

		if(!drives[i].is_chs_addressing) {
			_tty_printf("(%d bytes | %d KB | %d MB)",
					drives[i].capacity,
					drives[i].capacity >> 10,
					drives[i].capacity >> 20);
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

	ide_buf = (uint16_t*)kmalloc(512);
	
	register_interrupt_handler(32 + ATA_PRIMARY_IRQ, ide_primary_irq); // Fuck IRQs
	register_interrupt_handler(32 + ATA_SECONDARY_IRQ, ide_secondary_irq);

	ata_check_all();

	kfree(ide_buf);
}
