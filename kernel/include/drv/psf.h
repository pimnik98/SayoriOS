#pragma once

#include <common.h>

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

typedef struct __attribute__((packed)) {
    uint8_t magic[2];
    uint8_t mode;
    uint8_t charHeight;
} psf_t;

void draw_vga_str(const char* text, size_t len, int x, int y, int color);
size_t psf1_get_w();
size_t psf1_get_h();