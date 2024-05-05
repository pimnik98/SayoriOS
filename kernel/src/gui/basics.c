#include <common.h>
#include "io/screen.h"

void fill_screen(const uint32_t color) {
    const size_t w = getScreenWidth();
	const size_t h = getScreenHeight();

    for(int i = 0; i < h; i++) {
        for(int j = 0; j < w; j++) {
            set_pixel(j, i, color);
        }
    }
}

void draw_rectangle(const uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t color) {
    for(size_t i = x, to = x+w; i < to; i++) {
        set_pixel(i, y, color);
        set_pixel(i, y+h, color);
    }
	
    for(size_t j = y, to2 = y+h; j < to2; j++) {
        set_pixel(x, j, color);
        set_pixel(x+w, j, color);
    }
    
}

void draw_filled_rectangle(size_t x, size_t y, size_t w, size_t h, const uint32_t fill) {
    for(register int i = 0; i < h; i++) {
        for(register int j = 0; j < w; j++) {
            set_pixel(x+j, y+i, fill);
        }
    }
}
