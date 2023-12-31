#pragma once

#include <common.h>

void draw_line(int x0, int y0, int x1, int y1, int thickness, int color);
void draw_line_extern(uint8_t *buffer, size_t width, size_t height, int x0, int y0, int x1, int y1, int thickness, int color);