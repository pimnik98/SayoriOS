#include <common.h>
#include <fmt/wav.h>
#include "io/status_loggers.h"
#include "mem/vmm.h"
#include "drv/audio/ac97.h"
#include "sys/scheduler.h"
#include "sys/timer.h"
#include "lib/stdio.h"

char* miniplay_filename;
size_t miniplay_pages_total = 0;
size_t miniplay_pages_played = 0;
size_t miniplay_filesize = 0;
size_t miniplay_timestamp = 0;
WAVHeader miniplay_hdr;

size_t miniplay_anim_pos = 0;

// Duration = File Size / (Sample Rate * Number of Channels * Sample Width)

void miniplay_display() {
	while(1) {
		clean_tty_screen_no_update();

		_tty_printf("Miniplay by NDRAEY (c) 2023\n\n");
		_tty_printf("Now playing - %s\n\n", miniplay_filename);
		_tty_printf("Channels: %d\n", miniplay_hdr.numChannels);
		_tty_printf("Sample rate: %d\n", miniplay_hdr.sampleRate);
		_tty_printf("Byte rate: %d\n", miniplay_hdr.byteRate);
		_tty_printf("Bits per sample: %d\n\n", miniplay_hdr.bitsPerSample);

		size_t current_seconds_precise = (timestamp() - miniplay_timestamp) / 1000;
		size_t total_seconds = miniplay_filesize / (miniplay_hdr.sampleRate * miniplay_hdr.numChannels * (miniplay_hdr.bitsPerSample >> 3));

		_tty_printf("%02d:%02d / %02d:%02d\n\n",
					current_seconds_precise / 60,
					current_seconds_precise % 60,
					total_seconds / 60,
					total_seconds % 60);

		_tty_printf("%*s>     >     >     >\n", miniplay_anim_pos, "");
		_tty_printf("%*s<     <     <     <", 5 - miniplay_anim_pos, "");

		miniplay_anim_pos++;

		if(miniplay_anim_pos > 5) {
			miniplay_anim_pos = 0;
		}

		punch();

		sleep_ms(250);
	}
}

uint32_t miniplay(uint32_t argc, char* args[]) {
	if(argc < 1) {
		tty_error("No arguments!\n");
		return 1;
	}

	if(!ac97_is_initialized()) {
		tty_error("AC'97 is not initialized!");
		return 1;
	}

	miniplay_pages_total = 0;
	miniplay_pages_played = 0;
	miniplay_filesize = 0;
	memset(&miniplay_hdr, 0, sizeof miniplay_hdr);

	char* filename = args[1];
	miniplay_filename = filename;

	FILE* file = fopen(filename, "rb");

	if(!file) {
		tty_error("Failed to open a file!\n");
		return 1;
	}

	fseek(file, 0, SEEK_END);

	miniplay_filesize = ftell(file);

	fseek(file, 0, SEEK_SET);

	fread(file, 1, sizeof miniplay_hdr, &miniplay_hdr);

	fseek(file, 0xae, SEEK_SET);

	char* data = kmalloc(miniplay_filesize);
	fread(file, miniplay_filesize, 1, data);

	set_cursor_enabled(false);

    // Thread.
	thread_t* display_thread = thread_create(get_current_proc(), miniplay_display, 0x1000, true, false);

	ac97_set_pcm_sample_rate(miniplay_hdr.sampleRate);

	ac97_set_master_volume(2, 2, false);
	ac97_set_pcm_volume(2, 2, false);

	miniplay_timestamp = timestamp();

    ac97_WriteAll(data, miniplay_filesize);

	ac97_reset_channel();

	kfree(data);
	fclose(file);

	thread_exit(display_thread);

	clean_tty_screen();

	set_cursor_enabled(true);

	return 0;
}