#pragma once

#include <common.h>

enum {
	TGA_MODE_NO_IMG = 0,	///< No image data included.
	TGA_MODE_UC_CMI = 1,	///< Uncompressed, color-mapped images.
	TGA_MODE_UC_RGB = 2,	///< Uncompressed, RGB images.
	TGA_MODE_UC_BWI = 3,	///< Uncompressed, black and white images.
	TGA_MODE_RL_CMI = 9,	///< Runlength encoded color-mapped images.
	TGA_MODE_RL_RGB = 10,	///< Runlength encoded RGB images.
	TGA_MODE_CP_BWI = 11,	///< Compressed, black and white images.
	TGA_MODE_CP_HDR = 32,	///< Compressed color-mapped data, using Huffman, Delta, and runlength encoding.
	TGA_MODE_UC_PQR = 33,	///< Compressed color-mapped data, using Huffman, Delta, and runlength encoding.  4-pass quadtree-type process.
};

typedef struct {
  unsigned char image_id;             // must be zero
  unsigned char colormap;           // must be zero
  unsigned char image_type;           // must be 2
  unsigned short cmaporig;
  unsigned short cmaplen; // must be zero
  unsigned char cmapent;            // must be zero
  unsigned short x;                 // must be zero
  unsigned short y;                 // image's height
  unsigned short h;                 // image's height
  unsigned short w;                 // image's width
  unsigned char bpp;                // must be 32
  unsigned char image_descriptor;          // must be 40
} __attribute__((packed)) tga_header_t;

bool tga_extract_info(const char* path, tga_header_t* out_meta);
int tga_extract_pixels(const char* path, uint32_t* pixel_buf);