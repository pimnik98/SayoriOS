//
// Created by ndraey on 25.10.23.
//

#include <io/screen.h>
#include "io/rgb_image.h"
#include "io/ports.h"

/**
 * @brief Рисует изображение в виде массива байт в видеопамяти.
 * @param data Массив данных изображения
 * @param width Ширина изображения
 * @param height Высота изображения
 * @param bpp Количество бит на пиксель
 * @param sx Начальная координата X
 * @param sy Начальная координата Y
 */
void draw_rgb_image(const char *data, size_t width, size_t height, size_t bpp, int sx, int sy) {
	int x, y = 0;

    size_t bytes_pp = bpp >> 3;

	while(y < height) {
		x = 0;
		while(x < width) {
			int px = PIXIDX(width * bytes_pp, x * bytes_pp, y);

			char r = data[px];
			char g = data[px + 1];
			char b = data[px + 2];
			char a = data[px + 3];
			size_t color = ((r & 0xff) << 16) | ((g & 0xff) << 8) | (b & 0xff);

//			if((bytes_pp == 4 && a != 0) || bytes_pp == 3) {
				set_pixel(sx + x, sy + y, color);
//			} else {
//                qemu_log("FUCK! %x", a);
//            }

			x++;
		}
		y++;
	}
}

/**
 * @brief Скалирует изображение (расширяет или уменьшает размер изображения).
 * @param pixels Массив байт изображения в памяти
 * @param w1 Начальная ширина
 * @param h1 Начальная высота
 * @param w2 Конечная ширина
 * @param h2 Конечная высота
 * @param alpha Имеется ли альфа-канал?
 * @param out Выходной буффер изображения.
 */
void scale_rgb_image(const char* pixels, unsigned int w1, unsigned int h1, uint32_t w2, uint32_t h2, char alpha, char* out) {
	uint32_t scr_w = (w1<<16)/w2;
	uint32_t scr_h = (h1<<16)/h2;

	int x;
	int y = 0;

	uint32_t x2;
	uint32_t y2;

	char mod = alpha?4:3;
	while(y<h2) {
		x = 0;
		while(x<w2) {
			x2 = (x*scr_w)>>16;
			y2 = (y*scr_h)>>16;

			out[PIXIDX(w2*mod, x*mod, y)] = pixels[PIXIDX(w1*mod, x2*mod, y2)];
			out[PIXIDX(w2*mod, x*mod, y)+1] = pixels[PIXIDX(w1*mod, x2*mod, y2)+1];
			out[PIXIDX(w2*mod, x*mod, y)+2] = pixels[PIXIDX(w1*mod, x2*mod, y2)+2];
			if(alpha) {
				out[PIXIDX(w2*mod, x*mod, y)+3] = pixels[PIXIDX(w1*mod, x2*mod, y2)+3];
			}
			x++;
		}
		y++;
	}
}