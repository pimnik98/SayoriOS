#include <kernel.h>

uint32_t dan_view(int argc, char** argv) {
    if(argc == 0) {
        tty_printf("File not selected!\n");
        return 1;
    }

    FILE* file = fopen(argv[argc], "rb");

    if(!file) {
        tty_printf("Failed to open file %s (argc: %d)\n", argv[argc], argc);
        return 1;
    }

    fclose(file);

    tty_printf("Opening file...\n");

    DANDescriptor_t* descriptor = alloc_dan(argv[argc], true);

    if(!descriptor) {
        tty_printf("Not a DAN file!\n");
        return 1;
    }

    char* buffer = kmalloc(descriptor->framesize);

    uint32_t idx = 0;

    tty_printf("Ready!!!\n");

    uint32_t sx = (getScreenWidth() - descriptor->header.width) / 2;
    uint32_t sy = (getScreenHeight() - descriptor->header.height) / 2;

    set_cursor_enabled(false);

    clean_tty_screen();

    while(1) {

        read_frame_dan(descriptor, idx, buffer);

        duke_rawdraw(buffer, &(struct DukeImageMeta) {
            "DUKE",
            descriptor->header.width,
            descriptor->header.height,
            descriptor->framesize,
            descriptor->header.alpha
        }, (int)sx, (int)sy);

        idx = (idx + 1) % descriptor->header.frame_count;

        int key = getCharRaw();

		if(key == 129 || key == 1)
			break;

        punch();
    }

    free_dan(descriptor);
    kfree(buffer);
    
    set_cursor_enabled(true);
    
    return 0;
}