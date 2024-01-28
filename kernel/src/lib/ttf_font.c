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

    fseek(fp, 0, SEEK_END);

    size_t filesize = ftell(fp);

    fseek(fp, 0, SEEK_SET);

    font->data = kcalloc(filesize, 1);
    fread(fp, filesize, 1, font->data);

    stbtt_InitFont(&font->info, font->data, 0);

    stbtt_GetFontVMetrics(&font->info, &font->ascent, &font->descent, &font->linegap);

    font->user_size = 32;

    qemu_log("ASCENT: %d; DESCENT: %d", font->ascent, font->descent);

    fclose(fp);
}

void ttf_draw_char(ttf_font_t *font, char *buffer, int bwidth, int bheight, int sx, int sy, int character) {
    // TODO: Remove hardcoded values

    float scale = stbtt_ScaleForPixelHeight(&font->info, font->user_size);

    int ax;
    int lsb;

    stbtt_GetCodepointHMetrics(&font->info, character, &ax, &lsb);

    int c_x1, c_y1, c_x2, c_y2;
    stbtt_GetCodepointBitmapBox(&font->info, character, scale, scale, &c_x1, &c_y1, &c_x2, &c_y2);
    qemu_log("==> C_X1: %d, C_Y1: %d, C_X2: %d, C_Y2: %d", c_x1, c_y1, c_x2, c_y2);

    char* bitmap = calloc(100 * 100, 1);

    // TODO: Draw character at correct position

    stbtt_MakeCodepointBitmap(&font->info, bitmap, c_x2 - c_x1, font->user_size,
                              font->user_size, scale, scale, character);

    for(int y = 0; y < font->user_size; y++) {
        for(int x = 0; x < c_x2; x++) {
            if(bitmap[y * font->user_size + x] != 0) {
                set_pixel(sx + x, sy + y + ((font->user_size / 2) + 5 /* idk */ + c_y1), 0xFFFFFF);
            }
        }
    }

    kfree(bitmap);
}


void ttf_draw_string(ttf_font_t* font, char* buffer, int bwidth, int bheight, int sx, int sy, const char* string) {
    // TODO: Remove hardcoded values

    while(*string) {
        float scale = stbtt_ScaleForPixelHeight(&font->info, font->user_size);

        int ax;
        int lsb;

        stbtt_GetCodepointHMetrics(&font->info, *string, &ax, &lsb);

//        int c_x1, c_y1, c_x2, c_y2;
//        stbtt_GetCodepointBitmapBox(&font->info, *string, scale, scale, &c_x1, &c_y1, &c_x2, &c_y2);

        qemu_log("AX: %d; LSB: %d", ax, lsb);

//        ttf_draw_char(font, buffer, bwidth, bheight, sx, sy, *string);
        ttf_draw_char(font, buffer, bwidth, bheight, sx, sy, *string);


        sx += (int)(scale * ax);

        string++;
    }
}

void ttf_destroy(ttf_font_t* font) {
    free(font->data);
}