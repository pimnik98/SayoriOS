#include <kernel.h>

void draw_circle(int32_t xc, int32_t yc, int32_t radius, uint32_t color) {
    int32_t x = 0;
    int32_t y = radius;
    int32_t delta = 1 - 2 * radius;
    int32_t error = 0;

    while (y >= 0) {
        set_pixel(xc + x, yc + y, color);
        set_pixel(xc + x, yc - y, color);
        set_pixel(xc - x, yc + y, color);
        set_pixel(xc - x, yc - y, color);

        error = 2 * (delta + y) - 1;

        if ((delta < 0) && (error <= 0)) {
            delta += 2 * ++x + 1;
            continue;
        }

        if ((delta > 0) && (error > 0)) {
            delta -= 2 * --y + 1;
            continue;
        }

        delta += 2 * (++x - y--);
    }
}
