//
// Created by ndraey on 25.10.23.
//

#pragma once

#include "common.h"

#define PIXIDX(w, x, y) ((w) * (y) + (x))

void draw_rgb_image(const char *data, size_t width, size_t height, size_t bpp, int sx, int sy);
void scale_rgb_image(const char* pixels, uint32_t w1, uint32_t h1, int w2, int h2, char alpha, char* out);