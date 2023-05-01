/**
 * @file imaging.h
 * @brief Описания функций imaging.h
 * @version 0.3.2
 * @author Drew >_ (pikachu_andrey@vk.com)
 */

#pragma once

#include <common.h>
#define RGB_TO_UINT32(r, g, b) ((r&0xff)<<16)|((g&0xff)<<8)|(b&0xff)
#define DUKE_MAGIC "DUKE"

#define PIXIDX(w, x, y) (w * y + x)

/**
 * @brief Структура изображения Duke.
 * @param width - ширина
 * @param height - высота
 * @param data_length - длина всех "сырых" пикселей
 * @param alpha - указывает на наличие альфа слоя
 */

// Duke Image Header struct is 13 bytes long.
struct DukeImageMeta {
	uint8_t  magic[4];
    uint16_t width;
    uint16_t height;
    uint32_t data_length;
    uint8_t  alpha;
} __attribute__((packed));

typedef struct DukeImageMeta DukeHeader_t;

// Returns 0 if OK, 1 if ERR
char duke_draw_from_file(const char *filename, size_t sx, size_t sy);
char duke_get_image_metadata(char *filename, struct DukeImageMeta* meta);
void duke_get_image_data(char* filename, struct DukeImageMeta meta, void* buf);
unsigned int duke_calculate_bufsize(unsigned int width, unsigned int height, unsigned int alpha);
void duke_rawdraw(char* data, struct DukeImageMeta* meta, int sx, int sy);
void duke_scale(char* pixels, unsigned int w1, unsigned int h1, int w2, int h2, char alpha, char* out);
char duke_draw_scaled(char* filename, int width, int height, int x, int y);
int pixidx(int width, int x, int y);
