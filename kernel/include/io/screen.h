#pragma once

#include <common.h>

extern uint8_t* framebuffer_addr;
extern uint8_t* back_framebuffer_addr;
extern size_t framebuffer_size;

#define VESA_WIDTH  (getScreenWidth())
#define VESA_HEIGHT (getScreenHeight())


#define PACK_INTO_RGB(struct_px) ((struct_px.r & 0xff) << 16)  |\
                                 ((struct_px.g & 0xff) << 8) |\
                                  (struct_px.b & 0xff)

typedef struct rgba_struct {
	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t a;
} rgba_color;

typedef struct rgb_struct {
	uint8_t r;
	uint8_t g;
	uint8_t b;
} rgb_color;

typedef struct screen_pixel {
	rgb_color color;
	uint32_t x, y;
} screen_pixel;

enum colors  {
	VESA_BLACK = 0x000000,
	VESA_BLUE = 0x0000AA,
	VESA_GREEN = 0x00AA00,
	VESA_CYAN = 0x00AAAA,
	VESA_RED = 0xAA0000,
	VESA_MAGENTA = 0xAA00AA,
	VESA_YELLOW = 0xAA5500,
	VESA_LIGHT_GREY = 0xAAAAAA,
	VESA_DARK_GREY = 0x555555,
	VESA_LIGHT_BLUE = 0x5555FF,
	VESA_LIGHT_GREEN = 0x55FF55,
	VESA_LIGHT_CYAN = 0x55FFFF,
	VESA_LIGHT_RED = 0xFF5555,
	VESA_LIGHT_MAGENTA = 0xFF55FF,
	VESA_LIGHT_YELLOW = 0xffff55,
	VESA_WHITE = 0xFFFFFF,
};

typedef struct svga_mode_info {
	uint16_t attributes;
	uint8_t windowA, windowB;
	uint16_t granularity;
	uint16_t windowSize;
	uint16_t segmentA, segmentB;
	uint32_t winFuncPtr;
	uint16_t pitch;

	uint16_t screen_width, screen_height;
	uint8_t wChar, yChar, planes, bpp, banks;
	uint8_t memoryModel, bankSize, imagePages;
	uint8_t reserved0;

	// Color masks
	uint8_t readMask, redPosition;
	uint8_t greenMask, greenPosition;
	uint8_t blueMask, bluePosition;
	uint8_t reservedMask, reservedPosition;
	uint8_t directColorAttributes;

	uint32_t physbase;
	uint32_t offScreenMemOff;
	uint16_t offScreenMemSize;
	uint8_t reserved1[206];
} __attribute__ ((packed)) svga_mode_info_t;

#define punch() memcpy(framebuffer_addr, back_framebuffer_addr, framebuffer_size);
//#define punch() rect_copy(0, 0, VESA_WIDTH, VESA_HEIGHT)

uint32_t getDisplayPitch();
uint32_t getScreenWidth();
uint32_t getScreenHeight();
uint32_t getDisplaySize();
size_t getDisplayAddr();
size_t getFrameBufferAddr();
size_t getPixel(int32_t x, int32_t y);
void set_pixel(uint32_t x, uint32_t y, uint32_t color);
void rect_copy(int x, int y, int width, int height);