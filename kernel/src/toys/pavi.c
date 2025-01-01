#include "common.h"
#include "lib/stdio.h"
#include "io/tty.h"
#include "io/status_loggers.h"
#include "fmt/tga.h"
#include "mem/vmm.h"
#include "io/rgb_image.h"
#include "drv/input/keyboard.h"
#include "drv/input/keymap.h"
#include "io/ports.h"
#include "sys/pixfmt.h"

tga_header_t pavi_tga_header;
void* pavi_image_buffer = 0;
void* pavi_view_buffer = 0;

uint32_t pavi_view(uint32_t argc, char* argv[]) {
    memset(&pavi_tga_header, 0, sizeof pavi_tga_header);

	if(argc < 1) {
		tty_error("No arguments!\n");
		return 1;
	}

    size_t start_load = timestamp();

	FILE* file = fopen(argv[1], "rb");

	if(!file) {
		tty_printf("Failed to open file: %s!\n", argv[1]);
		return 1;
	}

	fclose(file);

    clean_tty_screen();
    tty_printf("Pavi - Opening '%s'...", argv[1]);

	tga_extract_info(argv[1], &pavi_tga_header);

    qemu_log("W: %d H: %d", pavi_tga_header.w, pavi_tga_header.h);

    clean_tty_screen();
    tty_printf("Pavi - Reading contents of '%s'...", argv[1]);

    pavi_image_buffer = kmalloc(pavi_tga_header.w * pavi_tga_header.h * 4);
    pavi_view_buffer = kmalloc(pavi_tga_header.w * pavi_tga_header.h * 4);

	tga_extract_pixels(argv[1], pavi_image_buffer);

    qemu_warn("Ended reading data");

    size_t nh = getScreenHeight();
    size_t nw = (size_t)((double)getScreenWidth() * ((double)pavi_tga_header.w / (double)pavi_tga_header.h));

    if(nw > pavi_tga_header.w || nh > pavi_tga_header.h) {
        nw = pavi_tga_header.w;
        nh = pavi_tga_header.h;
    }

    qemu_note("Colormap is: %x", pavi_tga_header.colormap);

    pixfmt_conv(pavi_image_buffer, 32, pavi_tga_header.w, pavi_tga_header.h, SCREEN_BGR, SCREEN_RGB);

    scale_rgb_image(pavi_image_buffer, pavi_tga_header.w, pavi_tga_header.h, nw, nh, 1, pavi_view_buffer);

    qemu_note("Ended scaling");

    size_t end_load = timestamp();

    clean_tty_screen();
    tty_printf("Pavi - '%s' [%dx%d] => [%dx%d] (Loaded in: %u ms) (Press ESC to exit)", argv[1], pavi_tga_header.w, pavi_tga_header.h, nw, nh, end_load - start_load);

    keyboardctl(KEYBOARD_ECHO, 0);

    draw_rgb_image(pavi_view_buffer, nw, nh, 32, (int)(getScreenWidth() - nw) / 2, 16 + (int)(getScreenHeight() - nh) / 2);

    punch();

    while(1) {
        int key = getCharRaw();

        if(key == KEY_ESC) {
            break;
        }
    }

    clean_tty_screen();

    keyboardctl(KEYBOARD_ECHO, 1);

    kfree(pavi_image_buffer);
    kfree(pavi_view_buffer);

	return 0;
}