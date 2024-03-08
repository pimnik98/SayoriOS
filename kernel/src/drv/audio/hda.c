//
// Created by ndraey on 21.01.24.
//

#include "drv/pci.h"
#include "drv/audio/hda.h"
#include "io/ports.h"
#include "io/tty.h"
#include "mem/pmm.h"
#include "mem/vmm.h"

uint8_t hda_bus = 0,
        hda_slot = 0,
        hda_func = 0;

uint16_t hda_vendor = 0,
         hda_device = 0;

uint32_t hda_addr = 0;

volatile uint32_t* hda_corb = 0;
size_t hda_corb_phys = 0;
volatile uint32_t* hda_rirb = 0;
size_t hda_rirb_phys = 0;

size_t hda_corb_entry_count = 0;
size_t hda_rirb_entry_count = 0;

size_t hda_corb_current = 1;
size_t hda_rirb_current = 1;

#define WRITE32(reg, value) *(volatile uint32_t*)(hda_addr + (reg)) = (value)
#define READ32(reg) (*(volatile uint32_t*)(hda_addr + (reg)))
#define WRITE16(reg, value) *(volatile uint16_t*)(hda_addr + (reg)) = (value)
#define READ16(reg) (*(volatile uint16_t*)(hda_addr + (reg)))
#define WRITE8(reg, value) *(volatile uint8_t*)(hda_addr + (reg)) = (value)
#define READ8(reg) (*(volatile uint8_t*)(hda_addr + (reg)))

#define VERB(codec, node, verb, command) ((codec << 28) | (node << 20) | (verb << 8) | (command))

#define REG_GCAP 0x00
#define REG_SSYNC 0x34
#define REG_DMA_LOW_POSITION_ADDR 0x70
#define REG_DMA_HIGH_POSITION_ADDR 0x74


void hda_init() {
    // Find devce by its class and subclass numbers.
    pci_find_device_by_class_and_subclass(4, 3, &hda_vendor, &hda_device, &hda_bus, &hda_slot, &hda_func);

    if(hda_vendor && hda_device) {
        qemu_ok("Found Intel HDA! (%x:%x)", hda_vendor, hda_device);
    } else {
        return;
    }

    // Read memory base address
    hda_addr = pci_read32(hda_bus, hda_slot, hda_func, 0x10) & ~0b1111;

    pci_enable_bus_mastering(hda_bus, hda_slot, hda_func);

    qemu_ok("HDA address: %x", hda_addr);
    tty_printf("HDA address: %x\n", hda_addr);

    // Map the registers into our address space (without caching, because memory-mapped regs should not be cached).
    map_pages(
            get_kernel_page_directory(),
            hda_addr,
            hda_addr,
            PAGE_SIZE,
            PAGE_WRITEABLE | PAGE_CACHE_DISABLE // PAGE_PRESENT is set automatically
    );

    // Reset the entire controller!
    hda_reset();

    tty_printf("HDA RESET OKAY!\n");

    // Read capabilities
    size_t data = READ16(REG_GCAP);

    size_t input_streams = (data >> 8) & 0b1111;
    size_t output_streams = (data >> 12) & 0b1111;

    tty_printf("HDA: I: %d; O: %d;\n", input_streams, output_streams);

    hda_disable_interrupts();

    //turn off dma position transfer
    WRITE32(REG_DMA_LOW_POSITION_ADDR, 0);
    WRITE32(REG_DMA_HIGH_POSITION_ADDR, 0);

    //disable synchronization
	WRITE32(REG_SSYNC, 0);
    // WRITE32(0x38, 0);

    //stop CORB and RIRB
    WRITE8(0x4C, 0x0);
    WRITE8(0x5C, 0x0);

    hda_corb = kmalloc_common(1024, PAGE_SIZE);
    hda_rirb = kmalloc_common(2048, PAGE_SIZE);
//    phys_set_flags(get_kernel_page_directory(), (virtual_addr_t)hda_corb, PAGE_PRESENT | PAGE_WRITEABLE | PAGE_CACHE_DISABLE);
//    phys_set_flags(get_kernel_page_directory(), (virtual_addr_t)hda_rirb, PAGE_PRESENT | PAGE_WRITEABLE | PAGE_CACHE_DISABLE);
    hda_corb_phys = virt2phys(get_kernel_page_directory(), (virtual_addr_t) hda_corb);
    hda_rirb_phys = virt2phys(get_kernel_page_directory(), (virtual_addr_t) hda_rirb);

    qemu_note("CORB: V%x => P%x", (size_t)hda_corb, hda_corb_phys);
    qemu_note("RIRB: V%x => P%x", (size_t)hda_rirb, hda_rirb_phys);

    memset((uint32_t*)hda_corb, 0, 1024);
    memset((uint32_t*)hda_rirb, 0, 1024);

    qemu_ok("Allocated memory for CORB and RIRB!");

    // Write CORB address
	WRITE32(0x40, (uint32_t)hda_corb_phys); // First 32 bits
	WRITE32(0x44, 0); // Last 32 bits (we are 32-bit, so we don't need it)

	hda_corb_entry_count = hda_calculate_entries(READ8(0x4E));

    tty_printf("HDA: CORB: %d entries\n", hda_corb_entry_count);

    // Reset read pointer
    WRITE16(0x4A, (1 << 15));
	while((READ16(0x4A) & (1 << 15)) != (1 << 15));

    WRITE16(0x4A, 0);
	while((READ16(0x4A) & (1 << 15)) != 0);

    WRITE16(0x48, 0);

    // RIRB

    WRITE32(0x50, (uint32_t)hda_rirb_phys); // First 32 bits
    WRITE32(0x54, 0); // Last 32 bits (we are 32-bit, so we don't need it)

    hda_rirb_entry_count = hda_calculate_entries(READ8(0x5E));

    tty_printf("HDA: RIRB: %d entries\n", hda_rirb_entry_count);

    // Reset write pointer
    WRITE16(0x58, (1 << 15));

    // Implement loop to check is WP ready

    sleep_ms(50);

    // Enable interrupts
    WRITE16(0x5A, 1);

	qemu_log("Starting engines");
    // Start!
    WRITE8(0x4C, (1 << 1) | 0);
    WRITE8(0x5C, (1 << 1) | 0);

	qemu_ok("Okay!");

    for(size_t codec = 0; codec < 16; codec++) {
        size_t id = hda_send_verb_via_corb_rirb(VERB(codec, 0, 0xf00, 0));

        if(id != 0) {
            tty_printf("FOUND CODEC: %x\n", id);
        }
    }
}

uint32_t hda_send_verb_via_corb_rirb(uint32_t verb) {
    qemu_warn("CWP: %d; CRP: %d; RWP: %d", READ16(0x48), READ16(0x4A), READ16(0x58));
    qemu_note("CORB CURRENT: %d", hda_corb_current);

    // SEND VERB
    hda_corb[hda_corb_current] = verb;

    qemu_note("WROTE %x TO CORB[%d]", verb, hda_corb_current);

    WRITE16(0x48, hda_corb_current & 0xff);

    while(true) {
        if(READ16(0x58) == hda_corb_current) {
            break;
        }
        qemu_warn("CWP: %d; CRP: %d; RWP: %d", READ16(0x48), READ16(0x4A), READ16(0x58));
        sleep_ms(1000);
    }

    // READ RESPONSE
    uint32_t response = hda_rirb[hda_rirb_current * 2];

    qemu_log("VERB %x got response %x", verb, response);

    hda_corb_current++;
    hda_rirb_current++;

    if(hda_corb_current == hda_corb_entry_count) {
        hda_corb_current = 1;
    }

    if(hda_rirb_current == hda_rirb_entry_count) {
        hda_rirb_current = 1;
    }

    return response;
}

void hda_reset() {
    if(!hda_vendor)
        return;

    WRITE32(0x8, 0);

    while((READ32(0x08) & 1) != 0);

    WRITE32(0x8, 1);

    while ((READ32(0x08) & 1) != 1);

    qemu_ok("Reset ok!");
}

size_t hda_calculate_entries(size_t word) {
    if((word & 0x40) == 0x40) {
        return 256;
    } else if((word & 0x20) == 0x20) {
        return 16;
    } else if((word & 0x10) == 0x10) {
        return 2;
    } else {
        return 0;
    }
}

void hda_disable_interrupts() {
    WRITE32(0x20, 0);
}
