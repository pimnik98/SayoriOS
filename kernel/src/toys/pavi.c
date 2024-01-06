#include "common.h"
#include "lib/stdio.h"
#include "io/tty.h"
#include "io/status_loggers.h"
#include "fmt/tga.h"

uint32_t pavi_view(uint32_t argc, char* argv[]) {
	if(argc < 1) {
		tty_error("No arguments!\n");
		return 1;
	}

	FILE* file = fopen(argv[1], "rb");

	if(!file) {
		tty_printf("Failed to open file: %s!\n", argv[1]);
		return 1;
	}

	fclose(file);

	tga_header_t pavi_tga_header;

	tga_extract_info(argv[1], &pavi_tga_header);



//	tga_extract_pixels(argv[1], )

	return 0;
}