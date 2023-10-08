//
// Created by ndraey on 10.09.23.
//

#include "sys/pixfmt.h"

void pixfmt_conv(char* pixels, size_t bpp, size_t width, size_t height, koraidon_screen_pixfmt_t input_format, koraidon_screen_pixfmt_t output_format) {
	if(input_format == output_format) {
		return;
	}

	size_t bytes_pp = bpp >> 3;

	bool in_rgb = input_format == SCREEN_RGB;
	bool in_bgr = input_format == SCREEN_BGR;

	bool out_rgb = output_format == SCREEN_RGB;
	bool out_bgr = output_format == SCREEN_BGR;

	for(size_t sy = 0; sy < height; sy++) {
		for(size_t sx = 0; sx < width; sx++) {
			size_t coords = (sy * (width * bytes_pp)) + (sx * bytes_pp);

			if((in_rgb && out_bgr) || (out_rgb && in_bgr)) {
				char pixel = pixels[coords + 2]; // r / b
				pixels[coords + 2] = pixels[coords + 0]; // b / r
				pixels[coords + 0] = pixel;
			}
		}
	}
}
