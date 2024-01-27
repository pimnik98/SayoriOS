//
// Created by ndraey on 27.01.24.
//

#pragma once

#include "3rdparty/stb_truetype.h"

typedef struct {
    unsigned char* data;

    stbtt_fontinfo info;
} ttf_font_t;

void ttf_init(ttf_font_t* font, const char* path);
void ttf_draw_char(ttf_font_t* font, char* buffer, int bwidth, int bheight, int character);
void ttf_destroy(ttf_font_t* font);