#pragma once
#include <stdint.h>
#include <stddef.h>
#define PSF_V1 0x01
#define PSF_V2 0x02

#define PSF1_MODE512    0X01
#define PSF1_MODEHASTAB 0X02
#define PSF1_MODEHASSEQ 0X04
#define PSF1_MAXMODE    0X05

#define PSF1_SEPARATOR  0XFFFF
#define PSF1_STARTSEQ   0XFFFE

#define PSF1_MAGIC0     0X36
#define PSF1_MAGIC1     0X04

/**
 * @brief Структура заголовка PSF шрифта.
 */
typedef struct {
    uint8_t magic[2];     /**< Магическое число PSF шрифта. */
    uint8_t mode;         /**< Режим PSF шрифта. */
    uint8_t charHeight;   /**< Высота символа в пикселях. */
} psf_t;

int psf1_init(char* psf);
size_t psf1_get_w();
size_t psf1_get_h(); 
void psf1_write_ch(uint16_t c, uint16_t c2, size_t pos_x, size_t pos_y, size_t color);
void psf1_write_str(const char* text, size_t len, int x, int y, uint32_t color);