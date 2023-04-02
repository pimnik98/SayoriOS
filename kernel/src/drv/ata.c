#include <kernel.h>

#define ATA_PRIMARY_IO 0x1F0
#define ATA_SECONDARY_IO 0x170

#define ATA_PRIMARY_DCR_AS 0x3F6
#define ATA_SECONDARY_DCR_AS 0x376

#define ATA_PRIMARY_IRQ 14
#define ATA_SECONDARY_IRQ 15

// PRIMARY MASTER
// PRIMARY SLAVE
// SECONDARY MASTER
// SECONDARY SLAVE
ata_drive_t drives[4] = {0};

uint16_t *ide_buf = 0;

void ide_select_drive(uint8_t bus, uint8_t i) {
	if(bus == ATA_PRIMARY)
		if(i == ATA_MASTER)
			outb(ATA_PRIMARY_IO + ATA_REG_HDDEVSEL, 0xA0);
		else
			outb(ATA_PRIMARY_IO + ATA_REG_HDDEVSEL, 0xB0);
	else
		if(i == ATA_MASTER)
			outb(ATA_SECONDARY_IO + ATA_REG_HDDEVSEL, 0xA0);
		else
			outb(ATA_SECONDARY_IO + ATA_REG_HDDEVSEL, 0xB0);
}

void ide_primary_irq() {}
void ide_secondary_irq() {}

uint8_t ide_identify(uint8_t bus, uint8_t drive) {
	uint16_t io = 0;
	
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

	if(status) {
		/* Now, poll untill BSY is clear. */
		while((inb(io + ATA_REG_STATUS) & ATA_SR_BSY) != 0) ;
pm_stat_read:		status = inb(io + ATA_REG_STATUS);
		
		qemu_log("Got status %x", status);
		if(status & ATA_SR_ERR) {
			qemu_log("%s %s has ERR set. Disabled.", PRIM_SEC(bus), MAST_SLV(drive));
			return 0;
		}

		while(!(status & ATA_SR_DRQ))
			goto pm_stat_read;
		
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
			if(!capacity_lba_ext)
				capacity = (cyl * hds * scs) + 128;
			else
				capacity = capacity_lba_ext;
		}

		drives[(bus << 1) | drive].capacity = capacity * 512;
	}else{
		qemu_log("%s %s => No status. Drive may be disconnected!", PRIM_SEC(bus), MAST_SLV(drive));
	}

	return 0;
}

inline void ide_400ns_delay(uint16_t io) {
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

	uint8_t status = inb(io + ATA_REG_STATUS);
	
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

uint8_t ata_write_raw_sector(uint8_t drive, uint8_t *buf, uint32_t lba) {
	// Only 28-bit LBA supported!
	lba &= 0x00FFFFFF;

	uint16_t io = 0;
	
	if(!drives[drive].online) {
		qemu_log("Attemted read from drive that does not exist.");
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
	for(size_t i = 0; i < sectors; i++) {
		ata_write_raw_sector(drive, buf + (i * 512), lba + i);
	}
}

void ata_read_sectors(uint8_t drive, uint8_t *buf, uint32_t lba, uint32_t numsects) {
	uint8_t* rbuf = buf;
	
	for(size_t i = 0; i < numsects; i++) {
		ata_read_sector(drive, rbuf, lba + i);
		rbuf += 512;
	}
}

// UNTESTED
void ata_read(uint8_t drive, uint8_t* buf, uint32_t location, uint32_t length) {
	if(!drives[drive].online) {
		qemu_log("Attempted read from drive that does not exist.");
		return;
	}
	
	size_t start_sector = location / 512;
	size_t end_sector = (location + length - 1) / 512;
	size_t sector_count = end_sector - start_sector + 1;
	
	size_t real_length = sector_count * 512;

	uint8_t* real_buf = kmalloc(real_length);

	ata_read_sectors(drive, real_buf, start_sector, sector_count);

	memcpy(buf, real_buf + (location % 512), length);

	kfree(real_buf);
}

void ata_write(uint8_t drive, const uint8_t* buf, size_t location, size_t length) {
	if(!drives[drive].online) {
		qemu_log("Attempted read from drive that does not exist.");
		return;
	}
	
	size_t start_sector = location / 512;
    size_t end_sector = (location + length - 1) / 512;
    size_t sector_count = end_sector - start_sector + 1;
    
    uint8_t* temp_buf = kmalloc(sector_count * 512);
    
    ata_read_sectors(drive, temp_buf, start_sector, sector_count);
    
    size_t start_offset = location % 512;
    memcpy(temp_buf + start_offset, buf, length);
    
    ata_write_sectors(drive, temp_buf, start_sector, sector_count);
    
    kfree(temp_buf);
}


void ata_list() {
	for (size_t i = 0; i < 4; i++) {
		tty_printf("ATA: %s %s:  %s (%d bytes | %d KB | %d MB)\n",
					PRIM_SEC((i >> 1) & 1),
					MAST_SLV(i & 1),
					drives[i].online?"online ":"offline",
					drives[i].capacity,
					drives[i].capacity >> 10,
					drives[i].capacity >> 20
		);
	}
	
}

ata_drive_t* ata_get_drives() {
	return drives;
}

void ata_init() {
	qemu_log("Checking for ATA drives");

	ide_buf = (uint16_t*)kmalloc(512);
	
	register_interrupt_handler(ATA_PRIMARY_IRQ, ide_primary_irq);
	register_interrupt_handler(ATA_SECONDARY_IRQ, ide_secondary_irq);

	ide_identify(ATA_PRIMARY, ATA_MASTER);
	ide_identify(ATA_PRIMARY, ATA_SLAVE);
	ide_identify(ATA_SECONDARY, ATA_MASTER);
	ide_identify(ATA_SECONDARY, ATA_SLAVE);

	kfree(ide_buf);
}
