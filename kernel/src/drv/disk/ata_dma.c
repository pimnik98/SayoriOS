#include "common.h"
#include "drv/disk/ata_dma.h"
#include "drv/pci.h"
#include "io/ports.h"
#include "mem/vmm.h"
#include "mem/pmm.h"
#include "drv/disk/ata.h"
#include "debug/hexview.h"
#include "lib/math.h"

#define ATA_PCI_VEN 0x8086
#define ATA_PCI_DEV 0x7010

#define ATA_DMA_READ 0xC8
#define ATA_DMA_WRITE 0xCA

uint8_t ata_busnum;
uint8_t ata_slot;
uint8_t ata_func;

uint16_t ata_dma_bar4;

prdt_t* ata_dma_prdt = 0;
size_t ata_dma_phys_prdt = 0;
size_t prdt_entry_count = 16;

extern ata_drive_t drives[4];


void ata_dma_init() {
    pci_find_device(ATA_PCI_VEN, ATA_PCI_DEV, &ata_busnum, &ata_slot, &ata_func);

    uint16_t devnum = pci_get_device(ata_busnum, ata_slot, ata_func);

    qemu_log("ATA DMA ID: %d (%x)", devnum, devnum);

    if(devnum == PCI_VENDOR_NO_DEVICE) {
        qemu_log("ATA DMA not found!");
        return;
    }else{
        qemu_log("Detected ATA DMA");
    }

    qemu_log("Enabling Busmastering");

    uint16_t command_register = pci_read_confspc_word(ata_busnum, ata_slot, ata_func, 4);
    command_register |= 0x05;
    pci_write(ata_busnum, ata_slot, ata_func, 4, command_register);

    qemu_log("Enabled Busmastering!!!");

    ata_dma_bar4 = pci_read_confspc_word(ata_busnum, ata_slot, ata_func, 0x20);

	if(ata_dma_bar4 & 0x01) {
		ata_dma_bar4 &= 0xfffffffc;
	}

    qemu_log("ATA DMA: BAR4: %x (%d)", ata_dma_bar4, ata_dma_bar4);

	qemu_log("PRDT: %d bytes", sizeof(prdt_t));

    // SETUP PRDT
	ata_dma_prdt = kmalloc_common(sizeof(prdt_t) * prdt_entry_count, PAGE_SIZE);
	memset(ata_dma_prdt, 0, sizeof(prdt_t) * prdt_entry_count);

	ata_dma_phys_prdt = virt2phys(get_kernel_page_directory(), (virtual_addr_t) ata_dma_prdt);

	qemu_log("PRDT ON: V%x; P%x", ata_dma_prdt, ata_dma_phys_prdt);
}

void ata_dma_clear_prdt() {
	memset(ata_dma_prdt, 0, sizeof(prdt_t) * prdt_entry_count);
}

void ata_dma_set_prdt_entry(prdt_t* prdt, uint16_t index, uint32_t address, uint16_t byte_count, bool is_last) {
    prdt[index].buffer_phys = address;
    prdt[index].transfer_size = byte_count;
    prdt[index].mark_end = is_last ? ATA_DMA_MARK_END : 0;
}

void dump_prdt(prdt_t* prdt) {
	int i = 0;
	size_t bytes = 0;
	qemu_warn("Dumping PRDT:");
	do {
		size_t size;

		if(prdt[i].transfer_size == 0)
			size = 65536;
		else
			size = prdt[i].transfer_size;

		qemu_log("[%d:%d] [Address: %x] -> %d", i, prdt[i].mark_end, prdt[i].buffer_phys, size);

		bytes += size;
		i++;
	} while(prdt[i - 1].mark_end != ATA_DMA_MARK_END);

	qemu_ok("Entries: %d; Bytes to process: %d", i, bytes);
}

status_t ata_dma_read_sector(uint8_t drive, uint8_t *buf, uint32_t lba) {
	ON_NULLPTR(buf, {
		qemu_err("Buffer is nullptr!");
		return E_INVALID_BUFFER;
	});

	if (!drives[drive].online) {
		qemu_err("Attempted read from drive that does not exist.");
		return E_DEVICE_NOT_ONLINE;
	}

	// Clear our prdt
	ata_dma_clear_prdt();

	// Allocate a temporary buffer
	void* temp_buf = kmalloc_common(PAGE_SIZE, PAGE_SIZE);
	memset(temp_buf, 0, PAGE_SIZE);

	// Make a physical address from a virtual to tell DMA where is our temp buffer
	size_t phys_buf = virt2phys(get_kernel_page_directory(), (virtual_addr_t) temp_buf);

	// Set only one PRDT entry to read only 512 bytes
	ata_dma_set_prdt_entry(ata_dma_prdt, 0, phys_buf, 512, true);

	// Only 28-bit LBA supported!
	lba &= 0x00FFFFFF;

	uint16_t io = 0;

	// Set our port address and drive number
	ata_set_params(drive, &io, &drive);

	// Set our addresses according to IO
	size_t status_offset = io == ATA_PRIMARY_IO ? ATA_DMA_PRIMARY_STATUS : ATA_DMA_SECONDARY_STATUS;
	size_t prdt_offset = io == ATA_PRIMARY_IO ? ATA_DMA_PRIMARY_PRDT : ATA_DMA_SECONDARY_PRDT;
	size_t cmd_offset = io == ATA_PRIMARY_IO ? ATA_DMA_PRIMARY_CMD : ATA_DMA_SECONDARY_CMD;

	outb(ata_dma_bar4 + cmd_offset, 8);
	outb(ata_dma_bar4 + status_offset, 6);

	// Send our PRDT address
	outl(ata_dma_bar4 + prdt_offset, ata_dma_phys_prdt);

	// Select drive
	outb(io + ATA_REG_HDDEVSEL, drive == ATA_MASTER ? 0xE0 : 0xF0);
	outb(io + 1, 0x00);

	// Write LBA
	outb(io + ATA_REG_LBA0, (uint8_t)((lba) & 0xFF));
	outb(io + ATA_REG_LBA1, (uint8_t)((lba >> 8) & 0xFF));
	outb(io + ATA_REG_LBA2, (uint8_t)((lba >> 16) & 0xFF));

	// Send command to read DMA!
	outb(io + ATA_REG_COMMAND, ATA_CMD_READ_DMA);

	// Start DMA transfer!
	outb(ata_dma_bar4 + cmd_offset, 9);

	while (1) {
		int status = inb(ata_dma_bar4 + status_offset);
		int dstatus = inb(io + ATA_REG_STATUS);

//		qemu_log("Status: %x; Dstatus: %x; ERR: %d", status, dstatus, status & (1 << 1));

		if (!(status & 0x04)) continue;
		if (!(dstatus & 0x80)) break;
	}

	outb(ata_dma_bar4 + cmd_offset, 0);

	memcpy(buf, temp_buf, 512);

	kfree(temp_buf);

	return OK;
}

// Internal function: do not use in production!
// WARNING: buffer is MUST be PAGE_SIZE aligned!
// WARNING: if numsects = 0 there's 256 sectors
status_t ata_dma_read_sectors(uint8_t drive, uint8_t *buf, uint32_t lba, uint8_t numsects) {
	ON_NULLPTR(buf, {
		qemu_err("Buffer is nullptr!");
		return E_INVALID_BUFFER;
	});

	if (!drives[drive].online) {
		qemu_err("Attempted read from drive that does not exist.");
		return E_DEVICE_NOT_ONLINE;
	}


	// Clear our prdt
	ata_dma_clear_prdt();

	// Make a physical address from a virtual to tell DMA where is our temp buffer
	size_t phys_buf = virt2phys(get_kernel_page_directory(), (virtual_addr_t)buf);
//	qemu_log("Read: buffer at %x (P%x); LBA: %d; %d sectors", buf, phys_buf, lba, numsects);

	// Fill the PRDT
	int i = 0;
	size_t byte_count;

	if(numsects == 0)
		byte_count = 256 * 512;
	else
		byte_count = numsects * 512;

//	qemu_warn("Filling with: %d bytes", byte_count);

	while(byte_count >= 65536) {
		byte_count -= 65536;

		ata_dma_set_prdt_entry(ata_dma_prdt, i, phys_buf + (i * 65536), 0, false);

		i++;
	}
//	qemu_warn("Remaining bytes: %d", byte_count);

	if(byte_count != 0) {
//		qemu_ok("Zero!");
		ata_dma_set_prdt_entry(ata_dma_prdt, i, phys_buf + (i * 65536), byte_count, true);
	} else {
//		qemu_ok("Not zero!");
		ata_dma_prdt[i - 1].mark_end = ATA_DMA_MARK_END;
	}

//	dump_prdt(ata_dma_prdt);

	// Only 28-bit LBA supported!
	lba &= 0x00FFFFFF;

	uint16_t io = 0;

	// Set our port address and drive number
	ata_set_params(drive, &io, &drive);

	// Set our addresses according to IO
	size_t status_offset = io == ATA_PRIMARY_IO ? ATA_DMA_PRIMARY_STATUS : ATA_DMA_SECONDARY_STATUS;
	size_t prdt_offset = io == ATA_PRIMARY_IO ? ATA_DMA_PRIMARY_PRDT : ATA_DMA_SECONDARY_PRDT;
	size_t cmd_offset = io == ATA_PRIMARY_IO ? ATA_DMA_PRIMARY_CMD : ATA_DMA_SECONDARY_CMD;

	outb(ata_dma_bar4 + cmd_offset, 8);
	outb(ata_dma_bar4 + status_offset, 6);

	// Send our PRDT address
	outl(ata_dma_bar4 + prdt_offset, ata_dma_phys_prdt);

	// Select drive
	outb(io + ATA_REG_HDDEVSEL, drive == ATA_MASTER ? 0xE0 : 0xF0);
	outb(io + 1, 0x00);

	// Write LBA
	outb(io + ATA_REG_SECCOUNT0, numsects);  // 0x00 is 128KB
	outb(io + ATA_REG_LBA0, lba & 0xFF);
	outb(io + ATA_REG_LBA1, (lba >> 8) & 0xFF);
	outb(io + ATA_REG_LBA2, (lba >> 16) & 0xFF);

	// Send command to read DMA!
	outb(io + ATA_REG_COMMAND, ATA_CMD_READ_DMA);

	// TODO: WAIT DRQ HERE

	// Start DMA transfer!
	outb(ata_dma_bar4 + cmd_offset, 9);

	while (1) {
		int status = inb(ata_dma_bar4 + status_offset);
		int dstatus = inb(io + ATA_REG_STATUS);

//		qemu_log("Status: %x; Dstatus: %x; ERR: %d", status, dstatus, status & (1 << 1));

		if (!(status & 0x04)) continue;
		if (!(dstatus & 0x80)) break;
	}

	outb(ata_dma_bar4 + cmd_offset, 0);

//	int status = inb(ata_dma_bar4 + status_offset);
//	int dstatus = inb(io + ATA_REG_STATUS);
//
//	qemu_log("FINAL: Status: %x; Dstatus: %x; ERR: %d", status, dstatus, status & (1 << 1));

	return OK;
}

// Internal function: do not use in production!
// WARNING: buffer is MUST be PAGE_SIZE aligned!
// WARNING: if numsects = 0 there's 256 sectors
status_t ata_dma_write_sectors(uint8_t drive, uint8_t *buf, uint32_t lba, uint8_t numsects) {
	ON_NULLPTR(buf, {
		qemu_err("Buffer is nullptr!");
		return E_INVALID_BUFFER;
	});

	if (!drives[drive].online) {
		qemu_err("Attempted read from drive that does not exist.");
		return E_DEVICE_NOT_ONLINE;
	}

	// Clear our prdt
	ata_dma_clear_prdt();

	// Make a physical address from a virtual to tell DMA where is our temp buffer
	size_t phys_buf = virt2phys(get_kernel_page_directory(), (virtual_addr_t)buf);
//	qemu_log("Read: buffer at %x (P%x); LBA: %d; %d sectors", buf, phys_buf, lba, numsects);

	// Fill the PRDT
	int i = 0;
	size_t byte_count;

	if(numsects == 0)
		byte_count = 256 * 512;
	else
		byte_count = numsects * 512;

//	qemu_warn("Filling with: %d bytes", byte_count);

	while(byte_count >= 65536) {
		byte_count -= 65536;

		ata_dma_set_prdt_entry(ata_dma_prdt, i, phys_buf + (i * 65536), 0, false);

		i++;
	}
//	qemu_warn("Remaining bytes: %d", byte_count);

	if(byte_count != 0) {
//		qemu_ok("Zero!");
		ata_dma_set_prdt_entry(ata_dma_prdt, i, phys_buf + (i * 65536), byte_count, true);
	} else {
//		qemu_ok("Not zero!");
		ata_dma_prdt[i - 1].mark_end = ATA_DMA_MARK_END;
	}

//	dump_prdt(ata_dma_prdt);

	// Only 28-bit LBA supported!
	lba &= 0x00FFFFFF;

	uint16_t io = 0;

	// Set our port address and drive number
	ata_set_params(drive, &io, &drive);

	// Set our addresses according to IO
	size_t status_offset = io == ATA_PRIMARY_IO ? ATA_DMA_PRIMARY_STATUS : ATA_DMA_SECONDARY_STATUS;
	size_t prdt_offset = io == ATA_PRIMARY_IO ? ATA_DMA_PRIMARY_PRDT : ATA_DMA_SECONDARY_PRDT;
	size_t cmd_offset = io == ATA_PRIMARY_IO ? ATA_DMA_PRIMARY_CMD : ATA_DMA_SECONDARY_CMD;

	outb(ata_dma_bar4 + cmd_offset, 0);
	outb(ata_dma_bar4 + status_offset, 6);

	// Send our PRDT address
	outl(ata_dma_bar4 + prdt_offset, ata_dma_phys_prdt);

	// Select drive
	outb(io + ATA_REG_HDDEVSEL, drive == ATA_MASTER ? 0xE0 : 0xF0);
	outb(io + 1, 0x00);

	// Write LBA
	outb(io + ATA_REG_SECCOUNT0, numsects);  // 0x00 is 128KB
	outb(io + ATA_REG_LBA0, lba & 0xFF);
	outb(io + ATA_REG_LBA1, (lba >> 8) & 0xFF);
	outb(io + ATA_REG_LBA2, (lba >> 16) & 0xFF);

	// Send command to write DMA!
	outb(io + ATA_REG_COMMAND, ATA_CMD_WRITE_DMA);

	// TODO: WAIT DRQ HERE

	// Start DMA transfer!
	outb(ata_dma_bar4 + cmd_offset, 1);

	while (1) {
		int status = inb(ata_dma_bar4 + status_offset);
		int dstatus = inb(io + ATA_REG_STATUS);

		qemu_log("Status: %x; Dstatus: %x; ERR: %d", status, dstatus, status & (1 << 1));

		if (!(status & 0x04)) continue;
		if (!(dstatus & 0x80)) break;
	}

	outb(ata_dma_bar4 + cmd_offset, 0);

	return OK;
}

status_t ata_dma_read(uint8_t drive, char *buf, uint32_t location, uint32_t length) {
	ON_NULLPTR(buf, {
		qemu_err("Buffer is nullptr!");
		return E_INVALID_BUFFER;
	});

	if(!drives[drive].online) {
		qemu_err("Attempted read from drive that does not exist.");
		return E_DEVICE_NOT_ONLINE;
	}

    qemu_log("DRIVE: %d; Buffer: %x, Location: %x, len: %d", drive, buf, location, length);

	size_t start_sector = location / drives[drive].block_size;
	size_t end_sector = (location + length - 1) / drives[drive].block_size;
	size_t sector_count = end_sector - start_sector + 1;

	size_t real_length = sector_count * drives[drive].block_size;

	// TODO: Optimize to read without kmalloc
	uint8_t* real_buf = kmalloc_common(real_length, PAGE_SIZE);

	if(!drives[drive].is_packet) {
		// Okay, ATA can only read 256 sectors (128 KB of memory) at one request, so subdivide our data to clusters to manage.
		int i = 0;
		size_t cluster_count = sector_count / 256;
		size_t remaining_count = sector_count % 256;

		for(; i < cluster_count; i++) {
			ata_dma_read_sectors(drive, real_buf + (i * (65536 * 2)), start_sector + (i * 256), 0);
		}

		if(remaining_count != 0)
			ata_dma_read_sectors(drive, real_buf + (i * (65536 * 2)), start_sector + (i * 256), remaining_count);
	}

//    hexview_advanced(real_buf, 512, 32, true, new_qemu_printf);

	memcpy(buf, real_buf + (location % drives[drive].block_size), length);

	kfree(real_buf);

	return OK;
}

status_t ata_dma_write(uint8_t drive, const char *buf, uint32_t location, uint32_t length) {
	ON_NULLPTR(buf, {
		qemu_err("Buffer is nullptr!");
		return E_INVALID_BUFFER;
	});

	if(!drives[drive].online) {
		qemu_log("Attempted read from drive that does not exist.");
		return E_DEVICE_NOT_ONLINE;
	}

	size_t start_sector = location / drives[drive].block_size;
	size_t end_sector = (location + length - 1) / drives[drive].block_size;
	size_t sector_count = end_sector - start_sector + 1;

	size_t real_length = sector_count * drives[drive].block_size;

	// TODO: Optimize to read without kmalloc
	uint8_t* real_buf = kmalloc_common(real_length, PAGE_SIZE);

	ata_dma_read_sectors(drive, real_buf, start_sector, sector_count);

	size_t start_offset = location % drives[drive].block_size;
	memcpy(real_buf + start_offset, buf, length);

	if(!drives[drive].is_packet) {
		// Okay, ATA can only operate with 256 sectors (128 KB of memory) at one request, so subdivide our data to clusters to manage.
		int i = 0;
		size_t cluster_count = sector_count / 256;
		size_t remaining_count = sector_count % 256;

		for(; i < cluster_count; i++) {
			ata_dma_write_sectors(drive, real_buf + (i * (65536 * 2)), start_sector + (i * 256), 0);
		}

		if(remaining_count != 0)
			ata_dma_write_sectors(drive, real_buf + (i * (65536 * 2)), start_sector + (i * 256), remaining_count);
	}

	kfree(real_buf);

	return OK;
}

