#include "common.h"
#include "drv/vfs_new.h"
#include "drv/input/keyboard.h"
#include "gui/basics.h"
#include "io/screen.h"
#include "io/tty.h"
#include "sys/trigger.h"
#include "drv/input/keymap.h"
#include "lib/string.h"
#include "drv/psf.h"
#include "io/ports.h"

char dino_cur_path[1024] = "/";
struct dirent* dino_cur_dirent = nullptr;
size_t dino_dirent_count = 0;
size_t dino_current_selection = 0;

void dino_draw_back() {
	draw_filled_rectangle(0, 0, VESA_WIDTH, VESA_HEIGHT, 0x444444);
	draw_filled_rectangle(40, 40, VESA_WIDTH - 80, VESA_HEIGHT - 80, 0x666666);
}

void dino_draw_panels() {

}

void dino_draw_main() {
	draw_rectangle(
			50,
			50 + (dino_current_selection * 16),
			strlen(dino_cur_dirent[dino_current_selection].name) * 8,
			16,
			0x000000
	);

	// TODO: Scrollable list
	for(int i = 0; i < dino_dirent_count; i++) {
		struct dirent elem = dino_cur_dirent[i];

		draw_vga_str(elem.name, strlen(elem.name), 50, 50 + (i * 16), 0x111111);
	}
}

void dino_update() {
	dino_cur_dirent = vfs_getListFolder(dino_cur_path);
	dino_dirent_count = vfs_getCountElemDir(dino_cur_path);
}

void dino_path_check() {
	// Remove trailing (duplictaing) slashes here.

	size_t len = strlen(dino_cur_path);
	size_t score = 0;

	for(int i = 0; i < len; i++) {
		if(dino_cur_path[i] == '/') {
			score++;

			if(score > 1) {
				memmove(dino_cur_path + i, dino_cur_path + i + 1, len - i);
			}
		} else {
			score = 0;
		}
	}
}

void dino_keyhandler(int key, int pressed, int c, int d, int e) {
	if (pressed) {
		if(key == KEY_DOWN)
			dino_current_selection = ((dino_current_selection + 1) % dino_dirent_count);
		else if(key == KEY_UP)
			dino_current_selection = dino_current_selection == 0 ? dino_dirent_count - 1 : dino_current_selection - 1;
		else if(key == KEY_ENTER) {
			qemu_log("You selected: %s", dino_cur_dirent[dino_current_selection].name);

			strcat(dino_cur_path, dino_cur_dirent[dino_current_selection].name);

			dino_path_check();

			qemu_log("Path: %s", dino_cur_path);

			dino_current_selection = 0;

			dino_update();
		}
	}
}

uint32_t dino_filemanager(int argc, char* argv[]) {
	dino_update();

	keyboardctl(KEYBOARD_ECHO, false);

	RegTrigger(TRIGGER_KEY_PRESSED, (trigger_cmd_t)dino_keyhandler);

	while(1) {
		int key = getCharRaw();
		if(key == KEY_ESC)
			break;

		dino_draw_back();
		dino_draw_panels();
		dino_draw_main();

		punch();
	}

	clean_tty_screen();

	keyboardctl(KEYBOARD_ECHO, true);

	return 0;
}
