#pragma once

#include <stdint.h>
uint32_t psp_display_combine_color_channels(uint8_t a, uint8_t r, uint8_t g, uint8_t b);
void psp_display_init();
uint32_t psp_display_width();
uint32_t psp_display_height();
void psp_display_clear(uint32_t color);
void psp_display_update();
void psp_display_set_pixel(unsigned int x, unsigned int y, uint32_t color);
void psp_display_draw_rect(unsigned int x, unsigned int y, unsigned int w, unsigned int h, uint32_t color);


#define display_width()  psp_display_width()
#define display_height() psp_display_height()
#define display_clear(color) psp_display_clear(color)
#define display_update() psp_display_update()
#define display_set_pixel(x, y, color) psp_display_set_pixel(x, y, color)