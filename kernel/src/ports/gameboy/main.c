#include "lib/stdio.h"
#include "timer.h"
#include "rom.h"
#include "mem.h"
#include "cpu.h"
#include "lcd.h"
#include "sdl.h"
#include "drv/input/keyboard.h"
#include "io/tty.h"

int gb_main(int argc, char *argv[]) {
	int r;
	const char usage[] = "Usage: %s <rom>\n";

	if(argc != 2) {
		printf(usage, argv[0]);
		return 0;
	}

	r = rom_load(argv[1]);
	if(!r)
		return 0;
	
	printf("ROM OK!\n");

	r = lcd_init();
	if(r)
		return 0;
	
	printf("LCD OK!\n");

	mem_init();
	printf("Mem OK!\n");

	cpu_init();
	printf("CPU OK!\n");

	keyboardctl(KEYBOARD_ECHO, false);
	set_cursor_enabled(false);

	r = 0;

	while(1)
	{
		int now;

		if(!cpu_cycle())
			break;

		now = cpu_get_cycles();

		while(now != r)
		{
			int i;

			for(i = 0; i < 4; i++)
				if(!lcd_cycle())
					goto out;

			r++;
		}

		timer_cycle();

		r = now;
	}
out:
	sdl_quit();

	keyboardctl(KEYBOARD_ECHO, true);
	set_cursor_enabled(true);

	return 0;
}
