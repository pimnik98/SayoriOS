//
// Created by ndraey on 08.11.23.
//

#include <common.h>
#include "drv/input/keymap.h"
#include "drv/input/keyboard.h"
#include "io/screen.h"
#include "sys/timer.h"
#include "lib/sprintf.h"

uint32_t gfxbench(uint32_t argc, char* args[]) {
	size_t frames = 0;
	size_t fps = 0;
	size_t last_measurement = timestamp();

	size_t scrw = getScreenWidth();
	size_t scrh = getScreenHeight();
	size_t scrbpp = getDisplayBpp();

	char* string = 0;

	keyboardctl(KEYBOARD_ECHO, false);

	while(1) {
		if(getCharRaw() == KEY_ESC)
			break;

		if(timestamp() - last_measurement >= 1000) {
			last_measurement = timestamp();
			fps = frames;
			frames = 0;
		}

		drawRect(0, 0, scrw, scrh, 0x999999);

		asprintf(&string, "[%d x %d @ %d bits] %d FPS", scrw, scrh, scrbpp, fps);

		draw_vga_str(string, strlen(string), 0, 0, 0x000000);

		if(string)
			kfree(string);

		punch();

		frames++;
	}

	keyboardctl(KEYBOARD_ECHO, true);

	return 0;
}