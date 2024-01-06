/**
 * @file fmt/tga.c
 * @author Пиминов Никита (nikita.piminoff@yandex.ru)
 * @brief [Images] Targa - Формат, который также используется многими игровыми движками (например, Quake)
 * @version 0.3.4
 * @date 2023-07-28
 * @copyright Copyright SayoriOS Team (c) 2022-2023
*/

#include "lib/stdio.h"
#include "mem/vmm.h"
#include "io/screen.h"
#include "io/tty.h"
#include <fmt/tga.h>
#include <io/ports.h>

int tga_paint(char* path) {
	ON_NULLPTR(path, {
		return 0;
	});

	FILE* file = fopen(path, "rb");
	qemu_log("[CMD] [TGA] [PAINT] Попытка открытия файла %s...\n", path);

	if (ferror(file) != 0){
		qemu_log("[CMD] [TGA] [PAINT] Не удалось найти файл `%s`. Проверьте правильность введенного вами пути.\n",path);
        return -1;
    }

//	rgba_color* rgba = kmalloc(sizeof(rgba_color));
	size_t zxfsize = fsize(file);

	char* ptr = kmalloc(zxfsize);
	fread(file, 1, zxfsize, ptr);

	tga_header_t* targa_header = (tga_header_t*)ptr;
	char* data = (char*)ptr + sizeof(tga_header_t);

	qemu_log("Image ID: %d", targa_header->image_id);
	qemu_log("Colormap type: %d", targa_header->colormap);
	qemu_log("Image type: %d", targa_header->image_type);
	qemu_log("Size: %dx%d", targa_header->w, targa_header->h);
	qemu_log("Bits per pixel: %d", targa_header->bpp);

	int i = 0,
		j = 0,
		k = 0,
		x = 0,
		y = 0,
		w = targa_header->w,
		h = targa_header->h,
		o = (ptr[11] << 8) + ptr[10];

	int m = ((ptr[1] ? (ptr[7]>>3)*ptr[5] : 0) + 18);
	
	if(w < 1 || h < 1) {
		qemu_log("[TGA] Invalid tga");
		return -2;
	}
	
	if (targa_header->image_type != 1
		&& targa_header->image_type != 2
		&& targa_header->image_type != 9
		&& targa_header->image_type != 10){
		qemu_log("[TGA] No support tga: %d",ptr[2]);
		return -3;
	}

	qemu_log("Data offset: %d", sizeof(tga_header_t));
	qemu_log("First 4 bytes: %x %x %x %x", data[0], data[1], data[2], data[3]);
	
	switch(targa_header->image_type) {
		case TGA_MODE_UC_CMI:
            if(ptr[6]!=0 || ptr[4]!=0 || ptr[3]!=0 || (ptr[7]!=24 && ptr[7]!=32)) {
				qemu_log("[TGA] Error tga#%d-4",ptr[2]);
				return -4;
			}
          
		    for(y=i=0; y<h; y++) {
                k = ((!o?h-y-1:y)*w);
                for(x=0; x<w; x++) {
                    j = ptr[m + k++]*(ptr[7]>>3) + 18;
                    //data[2 + i++] = ((ptr[7]==32?ptr[j+3]:0xFF) << 24) | (ptr[j+2] << 16) | (ptr[j+1] << 8) | ptr[j];
					
					char g = ptr[j];
					char a = ptr[j + 1];
					char b = ptr[j + 2];
					int r = (ptr[7]==32 ? ptr[j+3] : 0xff);
					int color = (	((a&0xff) << 24)  | 
									((g&0xff) << 16)  |
									((r&0xff) <<  8)  |
									(b&0xff));
					//data[2 + i++] = color;
					set_pixel(x,y,color);
                }
            }
            break;
		case TGA_MODE_UC_RGB:{
            if(ptr[5]!=0 || ptr[6]!=0 || ptr[1]!=0 || (ptr[16]!=24 && ptr[16]!=32)) {
				qemu_log("[TGA] Error tga#%d-4",ptr[2]);
				return -4;
			}
            for(y=i=0; y<h; y++) {
                j = ((!o?h-y-1:y)*w*(ptr[16]>>3));
                for(x=0; x < w; x++) {
				
					char g = ptr[j];
					char a = ptr[j+1];
					char b = ptr[j+2];
					int r = (ptr[16]==32?ptr[j+3]:0xff);
					int color = (	((a&0xff) << 24)  | 
									((g&0xff) << 16)  |
									((r&0xff) <<  8)  |
									(b&0xff));
					//data[2 + i++] = color;
					set_pixel(x,y,color);
                    j += ptr[16]>>3;
                }
            }
            break;
		}
		case TGA_MODE_RL_RGB: {
			if(ptr[5]!=0 || ptr[6]!=0 || ptr[1]!=0 || (ptr[16]!=24 && ptr[16]!=32))  {
				qemu_log("[TGA] Error tga#%d-4",ptr[2]);
				return -4;
			}

            y = i = 0;

//			size_t pixel = 0;
			size_t current_processing = 0;

			size_t cur_x = 0;
			size_t cur_y = 0;

			while(cur_y < targa_header->h) {
				uint8_t rep_count_raw = data[current_processing++];
				uint8_t repetition_count = (rep_count_raw & 0b01111111) + 1;
				
				// If packet is raw, repetition count will indicate pixel count.
				bool is_raw = !((rep_count_raw & 0b10000000) >> 7);  // Extract highest bit

				uint8_t r = 0,
						g = 0,
						b = 0,
						a = 0;
				
				if(!is_raw) {
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

						setPixelAlpha(cur_x, cur_y, (rgba_color) {
							b, g, r, a
						});

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

						setPixelAlpha(cur_x, cur_y, (rgba_color) {
							b, g, r, a
						});

						cur_x++;
					}

				}
				
			}
			break;
		}
		default:{
			qemu_log("[TGA] NO REASED %d",ptr[2]);
			break;
		}
	}

	qemu_log("[TGA] DONE %d-1",ptr[2]);
	punch();
	qemu_log("[TGA] DONE %d-2",ptr[2]);

	return 0;
}

int tga_info(char* path){
	FILE* file = fopen(path, "rb");
	qemu_log("[CMD] [TGA] Попытка открытия файла %s...\n", path);

	if (ferror(file) != 0){
		qemu_log("[CMD] [TGA] Не удалось найти файл `%s`. Проверьте правильность введенного вами пути.\n",path);
        return -1;
    }
	//uint32_t filesize = ftell(file);
	char* ptr = kmalloc(16);
	fread(file, 1, 16, ptr);

	int i = 0, j = 0, k = 0, x = 0, y = 0, w = (ptr[13] << 8) + ptr[12], h = (ptr[15] << 8) + ptr[14], o = (ptr[11] << 8) + ptr[10];
    int m = ((ptr[1]? (ptr[7]>>3)*ptr[5] : 0) + 18);

	if(w<1 || h<1) {
		qemu_log("[TGA] Invalid tga");
		return -2;
	}
	if (ptr[2] != 1 && ptr[2] != 2 && ptr[2] != 9 && ptr[2] != 10){
		qemu_log("[TGA] No support tga: %d",ptr[2]);
		return -3;
	}
	tty_printf("[TGA] Info:\n");
	tty_printf("[TGA] i:  %d\n",i);
	tty_printf("[TGA] j:  %d\n",j);
	tty_printf("[TGA] k:  %d\n",k);
	tty_printf("[TGA] x:  %d\n",x);
	tty_printf("[TGA] y:  %d\n",y);
	tty_printf("[TGA] w:  %d\n",w);
	tty_printf("[TGA] h:  %d\n",h);
	tty_printf("[TGA] o:  %d\n",o);
	tty_printf("[TGA] m:  %d\n",m);
	tty_printf("[TGA] mode:  %d\n",ptr[2]);
	
    kfree(ptr);
    fclose(file);
	return ptr[2];
} 