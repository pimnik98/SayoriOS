//
// Created by ndraey on 21.01.24.
//

#include "drv/pci.h"
#include "drv/audio/hda.h"
#include "io/ports.h"
#include "io/tty.h"
#include "mem/pmm.h"
#include "mem/vmm.h"
#include "sys/isr.h"

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

size_t hda_corb_current = 0;
size_t hda_rirb_current = 0;

size_t hda_irq = 0;

volatile bool hda_fired = false;
volatile size_t hda_response = 0;

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

size_t hda_afg_codec_id = 0;
size_t hda_afg_node_id = 0;

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

    uint32_t word = pci_read_confspc_word(hda_bus, hda_slot, hda_func, 0x3C);  // All 0xF PCI register
    hda_irq = word & 0xff;

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

    WRITE32(0x20, 0);

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
	while((READ16(0x4A) & (1 << 15)) != (1 << 15))
        ;

    WRITE16(0x4A, 0);
	while((READ16(0x4A) & (1 << 15)) != 0)
        ;

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
    WRITE16(0x5A, READ16(0x5A) | 1);
//    WRITE16(0x5A, 0xff);

	qemu_log("Starting engines");
    // Start!
    WRITE8(0x4C, (1 << 1) | 1);
    WRITE8(0x5C, (1 << 1) | 1);

    qemu_note("IRQ LINE IS: %d", hda_irq);

    register_interrupt_handler(32 + hda_irq, hda_interrupt_handler);

    WRITE32(0x20, (1 << 30) | (1 << 31));

    qemu_ok("Okay!");

    size_t codec_bitmap = READ16(0x0E);

    for(size_t codec = 0; codec < 16; codec++) {
        if(~codec_bitmap & (1 << codec)) {
            continue;
        }

        qemu_log("Probing codec: %d", codec);

        size_t id = hda_send_verb_via_corb_rirb(VERB(codec, 0, 0xf00, 0));

        if(id != 0) {
            tty_printf("FOUND CODEC: %x\n", id);

            hda_find_afg(id, codec);
        }
    }
}

void hda_find_afg(size_t codec_response, size_t codec_id) {
     hda_afg_codec_id = codec_id; // Save codec id
     
    // Read vendor id
    size_t vendor_id = (codec_response >> 16) & 0xffff;
    size_t dev_id = codec_response & 0xffff;

    tty_printf("|- Vendor: %x; Device: %x\n", vendor_id, dev_id);

    size_t child_nodes_info = hda_send_verb_via_corb_rirb(VERB(codec_id, 0, 0xf00, 0x04));
    size_t first_gnode = (child_nodes_info >> 16) & 0xff;
    size_t node_count = child_nodes_info & 0xff;
    size_t last_node = first_gnode + node_count;

    tty_printf("|- First group node: %d; Node count: %d\n", first_gnode, node_count);

    for(size_t node = first_gnode; node < last_node; node++) {
        size_t function_group_type = hda_send_verb_via_corb_rirb(VERB(codec_id, node, 0xf00, 0x05));

        if((function_group_type & 0x7f) == 0x01) {
            tty_printf("|- AFG at node: %d\n", node);
            tty_printf("|- FP: %d -> %d\n", codec_id, node);

            hda_afg_node_id = node; // Save node
            qemu_ok("UNBELIEVABLE! FOUND AFG!");

            hda_initialize_afg();
            break;
        }
    }
}

void hda_initialize_afg() {
    hda_send_verb_via_corb_rirb(VERB(hda_afg_codec_id, hda_afg_node_id, 0x7ff, 0));
    hda_send_verb_via_corb_rirb(VERB(hda_afg_codec_id, hda_afg_node_id, 0x705, 0));
    hda_send_verb_via_corb_rirb(VERB(hda_afg_codec_id, hda_afg_node_id, 0x708, 0));

    size_t hda_afg_node_sample_capabilities = hda_send_verb_via_corb_rirb(VERB(hda_afg_codec_id, hda_afg_node_id, 0xF00, 0x0A));
    size_t hda_afg_node_stream_format_capabilities = hda_send_verb_via_corb_rirb(VERB(hda_afg_codec_id, hda_afg_node_id, 0xF00, 0x0B));
    size_t hda_afg_node_input_amp_capabilities = hda_send_verb_via_corb_rirb(VERB(hda_afg_codec_id, hda_afg_node_id, 0xF00, 0x0D));
    size_t hda_afg_node_output_amp_capabilities = hda_send_verb_via_corb_rirb(VERB(hda_afg_codec_id, hda_afg_node_id, 0xF00, 0x12));

    qemu_note("hda_afg_node_sample_capabilities: %x", hda_afg_node_sample_capabilities);
    qemu_note("hda_afg_node_stream_format_capabilities: %x", hda_afg_node_stream_format_capabilities);
    qemu_note("hda_afg_node_input_amp_capabilities: %x", hda_afg_node_input_amp_capabilities);
    qemu_note("hda_afg_node_output_amp_capabilities: %x", hda_afg_node_output_amp_capabilities);


    size_t node_data = hda_send_verb_via_corb_rirb(VERB(hda_afg_codec_id, hda_afg_node_id, 0xf00, 0x04));
    size_t first_gnode = (node_data >> 16) & 0xff; // First group node
    size_t node_count = node_data & 0xff; // Node count
    size_t last_node = first_gnode + node_count;

    char* node_types[] = {
            "Output",
            "Input",
            "Mixer",
            "Selector",
            "Complex"
    };

    for(size_t node = first_gnode; node < last_node; node++) {
        qemu_note("node: %d", node);
        size_t type = (hda_send_verb_via_corb_rirb(VERB(hda_afg_codec_id, node, 0xF00, 0x09)) >> 20) & 0xF;

        if(type >= 5) {
            continue;
        }

        qemu_note("type: %d = %s", type, node_types[type]);
        tty_printf("|- %s at node: %d\n", node_types[type], node);

        if(type == 0x4) {
            size_t type_of_node = ((hda_send_verb_via_corb_rirb(VERB(hda_afg_codec_id, node, 0xF1C, 0x00)) >> 20) & 0xF);

            qemu_note("COMPLEX PIN HAS TYPE: %d", type_of_node);
            tty_printf("|- COMPLEX PIN HAS TYPE: %d\n", type_of_node);

            if(type_of_node == 0) {
                tty_printf("|- LINE OUT\n");
            } else if(type_of_node == 1) {
                tty_printf("|- SPEAKER\n");
            }
        }
    }
}

void hda_interrupt_handler(__attribute__((unused)) registers_t regs) {
//    qemu_warn("HDA Interrupt!");

    size_t interrupt_status = READ32(0x24);

    if(~interrupt_status & (1 << 31)) {
        return;
    }

    if(interrupt_status & (1 << 30)) {
        size_t rirb_status = READ8(0x5D);

        WRITE8(0x5D, rirb_status);

        if(rirb_status & (1 << 0)) {
            size_t rirb_wp = READ16(0x58);

//            qemu_ok("RESPONSE!");

            while(hda_rirb_current != rirb_wp) {
                hda_rirb_current = (hda_rirb_current + 1) % hda_rirb_entry_count;

                hda_fired = true;
                hda_response = hda_rirb[hda_rirb_current * 2];

//                qemu_ok("RIRB: %x", hda_rirb[hda_rirb_current * 2]);
            }
        }
    }
}

uint32_t hda_send_verb_via_corb_rirb(uint32_t verb) {
//    qemu_warn("CWP: %d; CRP: %d; RWP: %d", READ16(0x48), READ16(0x4A), READ16(0x58));

    hda_corb_current = (hda_corb_current + 1) % hda_corb_entry_count;

//    qemu_warn("CORB CUR: %d; RIRB CUR: %d", hda_corb_current, hda_rirb_current);
    // SEND VERB
    hda_corb[hda_corb_current] = verb;

    WRITE16(0x48, hda_corb_current & 0xff);

    while(!hda_fired)
        ;

    hda_fired = false;
    qemu_log("VERB %x got response %x", verb, hda_response);

    return hda_response;
}

void hda_reset() {
    if(!hda_vendor)
        return;

    WRITE32(0x8, READ32(0x8) & ~1);

    while((READ32(0x08) & 1) != 0);

    WRITE32(0x8, READ32(0x8) | 1);

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
