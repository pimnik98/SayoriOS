//
// Created by ndraey on 25.10.23.
//

#pragma once

#include "common.h"

#define PIXIDX(w, x, y) ((w) * (y) + (x))

void draw_rgb_image(const char *data, size_t width, size_t height, size_t bpp, int sx, int sy);
void scale_rgb_image(const char* pixels, unsigned int w1, unsigned int h1, uint32_t w2, uint32_t h2, char alpha, char* out);