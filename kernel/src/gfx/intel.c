// Intel HD Graphics (8086:2a42) driver by NDRAEY (c) 2024
// WARNING: Driver is in WIP STAGE
// For SayoriOS ;)

#include "drv/pci.h"
#include "io/ports.h"
#include "io/tty.h"
#include "mem/pmm.h"
#include "gfx/intel.h"


uint8_t igfx_bus = 0,
        igfx_slot = 0,
        igfx_func = 0;

size_t igfx_addr = 0;

size_t igfx_width = 0;
size_t igfx_height = 0;

#define IGFX_READ(reg) (*(volatile uint32_t*)(igfx_addr + reg))
#define IGFX_WRITE(reg, val) (*(volatile uint32_t*)(igfx_addr + reg) = val)

volatile size_t igfx_edid_buffer[32] = {0};

void igfx_gmbus_reset() {
    IGFX_WRITE(IGFX_GMBUS1, 0x80000000);
    IGFX_WRITE(IGFX_GMBUS1, 0);

    while(IGFX_READ(IGFX_GMBUS2) & (1 << 9))
        ;
}

void igfx_wait() {
    while (!(IGFX_READ(IGFX_GMBUS2) & (1 << 11)))
        ;
}

void igfx_init() {
    pci_find_device(0x8086, 0x2a42, &igfx_bus, &igfx_slot, &igfx_func);

    if(igfx_bus == 0xFF) {
        qemu_err("NO INTEL GFX!");
        tty_printf("NO INTEL GFX!\n");

        return;
    } else {
        qemu_ok("INTEL GFX!");
        tty_printf("INTEL GFX!\n");
    }

    pci_enable_bus_mastering(igfx_bus, igfx_slot, igfx_func);

    igfx_addr = pci_read32(igfx_bus, igfx_slot, igfx_func, PCI_BAR0) & ~0xF;

    map_pages(
        get_kernel_page_directory(),
        igfx_addr,
        igfx_addr,
        0x100000,
        PAGE_WRITEABLE | PAGE_CACHE_DISABLE
    );

    tty_printf("IGFX: %d.%d.%d AT %x\n", igfx_bus, igfx_slot, igfx_func, igfx_addr);


    // HERE GOES THE HELL

    size_t gmbus1 = (1 << 30) /* Ready */
                  | (1 << 26) /* Index used = 0 */
                  | (1 << 25) /* Cycle ends in WAIT */
                  | (128 << 16) /* EDID is 128 bytes long */
                  | (0x50 << 1) /* IDK why offset is 0x50 */
                  | 1; /* Read direction */

	tty_printf("RESETTING\n");

    igfx_gmbus_reset();
    
	tty_printf("OK\n");

    IGFX_WRITE(IGFX_GMBUS0, 3);
    IGFX_WRITE(IGFX_GMBUS1, gmbus1);

	tty_printf("Sent READ to controller!\n");

    for(int i = 0; i < 32; i++) {
        igfx_wait();

        uint32_t our_dword = IGFX_READ(IGFX_GMBUS3);

        igfx_edid_buffer[i] = our_dword;

        tty_printf("%x %x %x %x ",
                   (our_dword >> 24) & 0xff,
                   (our_dword >> 16) & 0xff,
                   (our_dword >> 8) & 0xff,
                   our_dword  & 0xff);
    }

    tty_printf("\nSTOPPING TRANSACTIONS\n");

    IGFX_WRITE(IGFX_GMBUS1, (1 << 30) | (1 << 27));

    tty_printf("\nOKAY!\n");



    uint8_t* data = (uint8_t*)igfx_edid_buffer;

    igfx_width = ((data[0x3a] >> 4) << 8) | (data[0x38]);
    igfx_height = ((data[0x3d] >> 4) << 8) | (data[0x3b]);

    tty_printf("[%x, %x, %x, %x]\n", data[0x3a], data[0x38], data[0x3d], data[0x3b]);
    tty_printf("WIDTH: %d; HEIGHT: %d\n", igfx_width, igfx_height);


    asm volatile("cli");


    // START
    IGFX_WRITE(IGFX_CURACNTR, IGFX_READ(IGFX_CURACNTR) & ~0x27);
    IGFX_WRITE(0x70084, 0);

    IGFX_WRITE(0x700c0, IGFX_READ(0x700c0) & ~0x27);
    IGFX_WRITE(0x700c4, 0);

    IGFX_WRITE(0x70183, IGFX_READ(0x70183) & ~0x80);
    IGFX_WRITE(0x70184, 0);

    IGFX_WRITE(0x71183, IGFX_READ(0x71183) & ~0x80);
    IGFX_WRITE(0x71184, 0);

    // MIDDLE

    IGFX_WRITE(0x70024, IGFX_READ(0x70024) | 0x2);
    IGFX_WRITE(0x71024, IGFX_READ(0x71024) | 0x2);

    IGFX_WRITE(0x7000B, IGFX_READ(0x7000B) & ~0x80); // disable pipe
    IGFX_WRITE(0x7100B, IGFX_READ(0x7100B) & ~0x80); // disable pipe

   while(IGFX_READ(0x7000B) & 0x40)
       ;

   while(IGFX_READ(0x7100B) & 0x40)
       ;

	size_t xaddr = 0x60000;

	if((IGFX_READ(0x61180) & (1 << 31)) == 0) {
		tty_printf("LVDS NOT ENABLED\n");
	} else {
		tty_printf("LVDS ENABLED\n");
		xaddr += 0x1000;
	}

	size_t command = ((igfx_width - 1) << 16) | (igfx_height - 1);
	size_t command_old = ((command & 0xffff) << 16) | ((command >> 16) & 0xffff);

	IGFX_WRITE(xaddr + 0x1C, command);
	IGFX_WRITE(xaddr + 0x10190, command_old);

	size_t scanline_w = ((igfx_width + 15) & ~15) << 2;

	IGFX_WRITE(xaddr + 0x10188, scanline_w);
	IGFX_WRITE(xaddr + 0x10184, 0);

    // END
	IGFX_WRITE(0x61233, IGFX_READ(0x61233) & ~0x80); // disable panel fitting
	IGFX_WRITE(xaddr + 0x1000B, IGFX_READ(0x7000B) | 0x80); // enable pipe
	IGFX_WRITE(xaddr + 0x10183, IGFX_READ(0x70183) | 0x80); // enable Display Plane A

    asm volatile("sti");
}
