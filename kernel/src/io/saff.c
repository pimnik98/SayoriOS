#include "../../include/kernel.h"
#include "../../include/io/saff.h"

SAFF_FMT_t* saff_read_metadata(char* raw) {
    return (SAFF_FMT_t*)raw;
}

void saff_display_data(SAFF_FMT_t* data) {
    tty_printf("MAGIC: %s\n", data->magic);
    tty_printf("PBYTE: %d\n", data->parameter_byte);
    tty_printf("FONT_CNT_BYTE: %d\n", data->font_count_byte);
    tty_printf("FONT_COUNT: %d\n", data->font_count);
    tty_printf("KERNING_BYTE: %d\n", data->kerning_byte);
    tty_printf("KERNING: %d\n", data->kerning);
    tty_printf("WIDTH: %d\n", data->width);
    tty_printf("HEIGHT: %d\n", data->height);
}

SAFF_LOAD_t* saff_load_font(char* filename) {
    if(!vfs_exists(filename)) return 0;

    unsigned int size = vfs_get_size(filename);

    SAFF_LOAD_t* dat = kheap_malloc(sizeof(SAFF_LOAD_t));
    memset(dat, 0, sizeof(SAFF_LOAD_t));

    dat->data = kheap_malloc(size);
    memset(dat->data, 0, size);

    dat->meta = kheap_malloc(sizeof(SAFF_FMT_t));
    memset(dat->meta, 0, sizeof(SAFF_FMT_t));

    // int32_t vfs_read(const char *filename, int32_t offset, int32_t size, void *buf);
    vfs_read(filename, 0, sizeof(SAFF_FMT_t), dat->meta);
    vfs_read(filename, sizeof(SAFF_FMT_t), size, dat->data);

    return dat;
}

void saff_destroy_font(SAFF_LOAD_t* font) {
    kheap_free(font->data);
    kheap_free(font->meta);
    kheap_free(font);
}