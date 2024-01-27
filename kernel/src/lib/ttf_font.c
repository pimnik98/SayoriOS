//
// Created by ndraey on 27.01.24.
//

#include "ttf_font.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "3rdparty/stb_truetype.h"

#include "lib/stdio.h"
#include "io/screen.h"
#include "io/ports.h"

void ttf_init(ttf_font_t* font, const char* path) {
    FILE* fp = fopen(path, "rb");

    if(!fp) {
        return;
    }

    size_t filesize = fp->size;

    font->data = kcalloc(filesize, 1);
    fread(fp, filesize, 1, font->data);


    stbtt_InitFont(&font->info, font->data, 0);

    fclose(fp);
}

void ttf_draw_char(ttf_font_t* font, char* buffer, int bwidth, int bheight, int character) {
    // TODO: Remove hardcoded values

    float scale = stbtt_ScaleForPixelHeight(&font->info, 64);

    char* bitmap = calloc(100 * 100, 1);

    stbtt_MakeCodepointBitmap(&font->info, bitmap, 100, 100,
                              100, scale, scale, 'Q');

    for(int y = 0; y < 100; y++) {
        for(int x = 0; x < 100; x++){
            if(bitmap[y * 100 + x] != 0) {
                set_pixel(x, y, 0xFFFFFFFF);
            }
        }
    }
}

void ttf_destroy(ttf_font_t* font) {
    free(font->data);
}