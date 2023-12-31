// TGA Decoder by pimnik98 and NDRAEY and OSDev Wiki

#include "common.h"
#include "lib/stdio.h"
#include "fmt/tga.h"
#include "io/ports.h"
#include "mem/vmm.h"

bool tga_extract_info(const char* path, tga_header_t* out_meta){
	FILE* file = fopen(path, "rb");

	if(!file){
        return false;
    }

    fread(file, sizeof(tga_header_t), 1, out_meta);

    fclose(file);

    return true;
}

// Pixel buffer needs to be 32-bit ARGB buffer
int tga_extract_pixels(const char* path, uint32_t* pixel_buf) {
	FILE* file = fopen(path, "rb");

	if(!file){
		return -1;
	}

	fseek(file, 0, SEEK_END);

	size_t filesize = ftell(file);

	fseek(file, 0, SEEK_SET);

	uint8_t* file_data = kcalloc(filesize, 1);

	fread(file, filesize, 1, file_data);

	tga_header_t* targa_header = (tga_header_t*)file_data;

	char* data = (char*)file_data + sizeof(tga_header_t);

	// IDK how to understand this code.

	int i = 0,
			j = 0,
			k = 0,
			x = 0,
			y = 0,
			w = targa_header->w,
			h = targa_header->h,
			o = (file_data[11] << 8) + file_data[10];

	int m = (targa_header->colormap ? (file_data[7]>>3) * file_data[5] : 0) + 18;

	if(w < 1 || h < 1) {
		return -2;
	}

	if (targa_header->image_type != 1
		&& targa_header->image_type != 2
		&& targa_header->image_type != 9
		&& targa_header->image_type != 10)
	{
		return -3;
	}

	switch(targa_header->image_type) {
		case TGA_MODE_UC_CMI: {
			if(file_data[6] != 0
			   || file_data[4] != 0
			   || file_data[3] != 0
			   || (file_data[7]!=24 && file_data[7]!=32)) {
				return -4;
			}

			for(y = i = 0; y<h; y++) {
				k = ((!o?h-y-1:y)*w);
				for(x=0; x<w; x++) {
					j = file_data[m + k++]*(file_data[7]>>3) + 18;

					int g = file_data[j];
					int a = file_data[j+1];
					int b = file_data[j+2];
					int r = (file_data[7]==32?file_data[j+3]:0xff);

					pixel_buf[
							y * targa_header->w + x
					] = (a << 24)
						| (b << 16)
						| (g << 8 )
						| (r << 0 );
				}
			}
			break;
		}
		case TGA_MODE_UC_RGB:{
			if(file_data[5]!=0 || file_data[6]!=0 || file_data[1]!=0 || (file_data[16]!=24 && file_data[16]!=32)) {
				return -4;
			}

			for(y = i = 0; y < h; y++) {
				j = (!o ? h - y - 1 : y) * w * (targa_header->bpp >> 3);
				for(x = 0; x < w; x++) {
					int g = file_data[j];
					int a = file_data[j+1];
					int b = file_data[j+2];
					int r = (targa_header->bpp == 32 ? file_data[j+3] : 0xff);

					pixel_buf[
							y * targa_header->w + x
					] = (a << 24)
						| (b << 16)
						| (g << 8 )
						| (r << 0 );

					j += targa_header->bpp >> 3;
				}
			}
			break;
		}
		case TGA_MODE_RL_RGB: {
			if(file_data[5]!=0 || file_data[6]!=0 || file_data[1]!=0 || (file_data[16]!=24 && file_data[16]!=32))  {
				return -4;
			}

			y = i = 0;

			size_t current_processing = 0;

			size_t cur_x = 0;
			size_t cur_y = 0;

			while(cur_y < targa_header->h) {
				uint8_t rep_count_raw = data[current_processing++];
				uint8_t repetition_count = (rep_count_raw & 0b01111111) + 1;

				// If packet is raw, repetition count will indicate pixel count.
				bool is_raw = ((rep_count_raw & 0b10000000) >> 7);  // Extract highest bit

				uint8_t r = 0,
						g = 0,
						b = 0,
						a = 0;

				if(is_raw) {
					r = data[current_processing++] & 0xff;
					g = data[current_processing++] & 0xff;
					b = data[current_processing++] & 0xff;

					if(targa_header->bpp == 32)
						a = data[current_processing++] & 0xff;
					else
						a = 0xFF;

					while(repetition_count--) {
						if(cur_x >= targa_header->w) {
							cur_x = 0;
							cur_y++;
						}

						// setPixelAlpha(cur_x, cur_y, (rgba_color) {
						// 	b, g, r, a
						// });

						pixel_buf[
								cur_y * targa_header->w + cur_x
						] = (a << 24)
							| (b << 16)
							| (g << 8 )
							| (r << 0 );

						cur_x++;
					}
				} else {
					while(repetition_count--) {
						r = data[current_processing++] & 0xff;
						g = data[current_processing++] & 0xff;
						b = data[current_processing++] & 0xff;

						if(targa_header->bpp == 32)
							a = data[current_processing++];
						else
							a = 0xFF;

						if(cur_x >= targa_header->w) {
							cur_x = 0;
							cur_y++;
						}

						// setPixelAlpha(cur_x, cur_y, (rgba_color) {
						// 	b, g, r, a
						// });

						pixel_buf[
								cur_y * targa_header->w + cur_x
						] = (a << 24)
							| (b << 16)
							| (g << 8 )
							| (r << 0 );

						cur_x++;
					}

				}
			}

			break;
		}
		default: {
			qemu_err("Error: Mode not implemented: %d\n", targa_header->image_type);
			break;
		}
	}

	kfree(file_data);
	fclose(file);

	return 0;
}

// Pixel buffer needs to be 32-bit ARGB buffer
int tga_extract_pixels_from_data(const char* raw_data, uint32_t* pixel_buf) {
	ON_NULLPTR(raw_data, {
		return 0;
	});

	tga_header_t* targa_header = (tga_header_t*)raw_data;

	qemu_warn("W: %d", targa_header->w);
	qemu_warn("H: %d", targa_header->h);
	qemu_warn("BPP: %d", targa_header->bpp);

	char* data = (char*)raw_data + sizeof(tga_header_t);

	// IDK how to understand this code.

	int i = 0,
			j = 0,
			k = 0,
			x = 0,
			y = 0,
			w = targa_header->w,
			h = targa_header->h,
			o = (raw_data[11] << 8) + raw_data[10];

	int m = (targa_header->colormap ? (raw_data[7]>>3) * raw_data[5] : 0) + 18;

	if(w < 1 || h < 1) {
		return -2;
	}

	if (targa_header->image_type != 1
		&& targa_header->image_type != 2
		&& targa_header->image_type != 9
		&& targa_header->image_type != 10)
	{
		return -3;
	}

	switch(targa_header->image_type) {
		case TGA_MODE_UC_CMI: {
			if(raw_data[6] != 0
			   || raw_data[4] != 0
			   || raw_data[3] != 0
			   || (raw_data[7]!=24 && raw_data[7]!=32)) {
				return -4;
			}

			for(y = i = 0; y<h; y++) {
				k = ((!o?h-y-1:y)*w);
				for(x=0; x<w; x++) {
					j = raw_data[m + k++]*(raw_data[7]>>3) + 18;

					int g = raw_data[j];
					int a = raw_data[j+1];
					int b = raw_data[j+2];
					int r = (raw_data[7]==32?raw_data[j+3]:0xff);

					pixel_buf[
							y * targa_header->w + x
					] = (a << 24)
						| (b << 16)
						| (g << 8 )
						| (r << 0 );
				}
			}
			break;
		}
		case TGA_MODE_UC_RGB:{
			if(raw_data[5]!=0 || raw_data[6]!=0 || raw_data[1]!=0 || (raw_data[16]!=24 && raw_data[16]!=32)) {
				return -4;
			}

			for(y = i = 0; y < h; y++) {
				j = (!o ? h - y - 1 : y) * w * (targa_header->bpp >> 3);
				for(x = 0; x < w; x++) {
					int g = raw_data[j];
					int a = raw_data[j+1];
					int b = raw_data[j+2];
					int r = (targa_header->bpp == 32 ? raw_data[j+3] : 0xff);

					pixel_buf[
							y * targa_header->w + x
					] = (a << 24)
						| (b << 16)
						| (g << 8 )
						| (r << 0 );

					j += targa_header->bpp >> 3;
				}
			}
			break;
		}
		case TGA_MODE_RL_RGB: {
			if(raw_data[5]!=0 || raw_data[6]!=0 || raw_data[1]!=0 || (raw_data[16]!=24 && raw_data[16]!=32))  {
				return -4;
			}

			y = i = 0;

			size_t current_processing = 0;

			size_t cur_x = 0;
			size_t cur_y = 0;

			while(cur_y < targa_header->h) {
				uint8_t rep_count_raw = data[current_processing++];
				uint8_t repetition_count = (rep_count_raw & 0b01111111) + 1;

				// If packet is raw, repetition count will indicate pixel count.
				bool is_raw = ((rep_count_raw & 0b10000000) >> 7);  // Extract highest bit

				uint8_t r = 0,
						g = 0,
						b = 0,
						a = 0;

				if(is_raw) {
					r = data[current_processing++] & 0xff;
					g = data[current_processing++] & 0xff;
					b = data[current_processing++] & 0xff;

					if(targa_header->bpp == 32)
						a = data[current_processing++] & 0xff;
					else
						a = 0xFF;

					while(repetition_count--) {
						if(cur_x >= targa_header->w) {
							cur_x = 0;
							cur_y++;
						}

						// setPixelAlpha(cur_x, cur_y, (rgba_color) {
						// 	b, g, r, a
						// });

						pixel_buf[
								cur_y * targa_header->w + cur_x
						] = (a << 24)
							| (b << 16)
							| (g << 8 )
							| (r << 0 );

						cur_x++;
					}
				} else {
					while(repetition_count--) {
						r = data[current_processing++] & 0xff;
						g = data[current_processing++] & 0xff;
						b = data[current_processing++] & 0xff;

						if(targa_header->bpp == 32)
							a = data[current_processing++];
						else
							a = 0xFF;

						if(cur_x >= targa_header->w) {
							cur_x = 0;
							cur_y++;
						}

						// setPixelAlpha(cur_x, cur_y, (rgba_color) {
						// 	b, g, r, a
						// });

						pixel_buf[
								cur_y * targa_header->w + cur_x
						] = (a << 24)
							| (b << 16)
							| (g << 8 )
							| (r << 0 );

						cur_x++;
					}

				}
			}

			break;
		}
		default: {
			qemu_log("Error: Mode not implemented: %d\n", targa_header->image_type);
			break;
		}
	}

	return 0;
}
