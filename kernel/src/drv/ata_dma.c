#include <kernel.h>

#define ATA_PCI_VEN 0x8086
#define ATA_PCI_DEV 0x7010

#define ATA_DMA_READ 0xC8
#define ATA_DMA_WRITE 0xCA

uint8_t ata_busnum;
uint8_t ata_slot;
uint8_t ata_func;

uint16_t ata_dma_bar4;

typedef struct prdt {
	uint32_t buffer_phys;
	uint16_t transfer_size;
	uint16_t mark_end;
}__attribute__((packed)) prdt_t;

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

    ata_dma_bar4--;

    qemu_log("ATA DMA: BAR4: %x (%d)", ata_dma_bar4, ata_dma_bar4);
}

prdt_t* ata_dma_allocate_prdt() {
    return kcalloc(1, sizeof(prdt_t));
}

void ata_dma_set_prdt_entry(prdt_t* prdt, uint16_t index, uint32_t address, uint16_t byte_count, bool is_last) {
    prdt[index].buffer_phys = address;
    prdt[index].transfer_size = byte_count;
    prdt[index].mark_end = is_last ? (0x8000) : (0x0000);
}

void ata_dma_test() {
    // SETUP PRDT

    prdt_t* prdt = kmalloc_common(sizeof(prdt_t), true);
    memset(prdt, 0, sizeof(prdt_t));

    size_t phys_prdt = virt2phys(get_kernel_dir(), prdt);
    
    qemu_log("PRDT: V%x => P%x", prdt, phys_prdt);
    
    // SETUP BUFFER

    size_t buf = kmalloc_common(PAGE_SIZE, true);
    memset(buf, 0, PAGE_SIZE);

    size_t phys_buf = virt2phys(get_kernel_dir(), buf);

    qemu_log("Buffer: V%x => P%x", buf, phys_buf);

    ata_dma_set_prdt_entry(prdt, 0, phys_buf, PAGE_SIZE, true);

    qemu_log("Address (physical): %x", prdt[0].buffer_phys);
    qemu_log("Size of transfer (bytes): %d", prdt[0].transfer_size);
    qemu_log("Last: %s", prdt[0].mark_end ? "yes" : "no");

    outl(ata_dma_bar4, 0);

    outl(ata_dma_bar4 + ATA_DMA_PRIMARY_PRDT, phys_prdt);
	// outb(ATA_PRIMARY_IO + ATA_REG_HDDEVSEL, (0xe0 | (0 & 0x0f000000) >> 24));
    ide_select_drive(ATA_PRIMARY, ATA_MASTER);

    outb(ATA_PRIMARY_IO + ATA_REG_SECCOUNT0, 1);

	outb(ATA_PRIMARY_IO + ATA_REG_LBA0, 0);
	outb(ATA_PRIMARY_IO + ATA_REG_LBA1, 0);
	outb(ATA_PRIMARY_IO + ATA_REG_LBA2, 0);

    outb(ATA_PRIMARY_IO + ATA_REG_COMMAND, ATA_CMD_READ_DMA);

    outl(ata_dma_bar4, 0x8 | 0x1);

    while (1) {
        int status = inb(ata_dma_bar4 + ATA_DMA_PRIMARY_STATUS);
        int dstatus = inb(ATA_PRIMARY_IO + ATA_REG_STATUS);

        qemu_log("Status: %x; Dstatus: %x", status, dstatus);

        if (!(status & 0x04)) {
            continue;
        }

        if (!(dstatus & 0x80)) {
            break;
        }
    }

    qemu_log("==============");
    qemu_log("Buffer: %s", buf);
    
    hexview_advanced(buf, 512, 16, true, new_qemu_printf);

    qemu_log("Ok?");



    kfree(buf);
    kfree(prdt);
}