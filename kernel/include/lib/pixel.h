#pragma once

#ifndef SAYORI_PIXEL_H
#define SAYORI_PIXEL_H

void drawRect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t color);
void drawRectLine(int x,int y,int w, int h,int color,int color2, int c);
void drawRectBorder(int x, int y, int w, int h, int color);
void drawHorizontalLine(int x1, int x2, int y, uint32_t color);
void drawVerticalLine(int y1, int y2, int x, uint32_t color);
void drawCirclePoints(int cx, int cy, int x, int y, uint32_t color);
void drawCircle(int cx, int cy, int radius, uint32_t color);
void drawFilledCircle(int x0, int y0, int radius, uint32_t color);
void drawFilledRectBorder(int x0, int y0, int radius, int w, int mode, uint32_t color);
void drawRoundedSquare(int x, int y, int size, int radius, uint32_t fill_color, uint32_t border_color);
void drawRoundedRectangle(int x, int y, int width, int height, int radius, uint32_t fill_color, uint32_t border_color);


#endif //SAYORI_PIXEL_H
