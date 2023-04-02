#include <common.h>
#include <kernel.h>

void fill_screen(uint32_t color) {
    size_t w = getWidthScreen();
    
    for(int i = 0, h = getHeightScreen(); i < h; i++) {
        for(int j = 0; j < w; j++) {
            set_pixel(j, i, color);
        }
    }
}

void draw_rectangle(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t color) {    
    for(int i = x, to = x+w; i < to; i++) {
        set_pixel(i, y, color);
        set_pixel(i, y+h, color);
    }
	
    for(int j = y, to2 = y+h; j < to2; j++) {
        set_pixel(x, j, color);
        set_pixel(x+w, j, color);
    }
    
}

void draw_filled_rectangle(size_t x, size_t y, size_t w, size_t h, uint32_t fill) {	
    for(int i = 0; i < h; i++) {
        for(int j = 0; j < w; j++) {
            set_pixel(x+j, y+i, fill);
        }
    }
}