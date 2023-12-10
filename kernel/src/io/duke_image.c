/**
 * @file io/imaging.c
 * @author NDRAEY >_ (pikachu_andrey@vk.com)
 * @brief Встраиваемая библиотека рисования изображений формата Duke
 * @version 0.3.3
 * @date 2022-11-01
 * @copyright Copyright SayoriOS Team (c) 2022-2023
 */

#include <drv/vfs_new.h>
#include <sys/memory.h>
#include <io/screen.h>
#include <lib/stdio.h>
#include <io/duke_image.h>
#include <io/ports.h>

/**
 * @brief Получает метаданные изображения Duke.
 *
 * @param filename - Имя файла
 *
 * @return 0 при успехе, 1 при ошибке
 */
char duke_get_image_metadata(char *filename, struct DukeImageMeta* meta) {
    if(vfs_exists(filename)) {
        int mount_addr_ = vfs_foundMount(filename);
        int file_index_ = vfs_findFile(filename);
        vfs_read(mount_addr_, file_index_, 0, sizeof(struct DukeImageMeta), meta);
        return 0;
    }
    return 1;
}

void duke_get_image_data(char* filename, struct DukeImageMeta meta, void* buf) {
    if(vfs_exists(filename)) {
        int mount_addr_ = vfs_foundMount(filename);
        int file_index_ = vfs_findFile(filename);
        vfs_read(mount_addr_, file_index_, sizeof(struct DukeImageMeta), meta.data_length, buf);
    }
}

char duke_draw_area_from_file(char *filename, int x, int y, int sx, int sy, int width, int height) {
    if(vfs_exists(filename)) {
        struct DukeImageMeta* meta = kmalloc(sizeof *meta);

        int mount_addr_ = vfs_foundMount(filename);
        int file_index_ = vfs_findFile(filename);
        vfs_read(mount_addr_, file_index_, 0, sizeof(struct DukeImageMeta), meta);

        if(width>meta->width) { width = meta->width; }
        if(height>meta->height) { height = meta->height; }

        char *imagedata = kmalloc(meta->data_length);

        vfs_read(mount_addr_, file_index_, sizeof(struct DukeImageMeta), meta->data_length, imagedata);

        int wx;
		int wy = sy;
        char mod = meta->alpha?4:3;

        while(wy<(height-sy)) {
            wx = sx;
            while((wx-sx)<(width)) {
                int px = PIXIDX(meta->width*mod, wx*mod, wy);
                char r = imagedata[px];
                char g = imagedata[px+1];
                char b = imagedata[px+2];
                char a = imagedata[px+3];
                int color = RGB_TO_UINT32(r, g, b);
                if(mod==4) {
                    if(a != 0) {
                        set_pixel(x+(wx-sx), y+(wy-sy), color);
                    }
                }else{
                    set_pixel(x+(wx-sx), y+(wy-sy), color);
                }
                wx++;
            }
            wy++;
        }
        kfree(imagedata);
        kfree(meta);
    }else{ return 1; }
    return 0;
}



/**
 * @brief Функция отрисовки изображения
 *
 * @param filename - Имя файла
 * @param sx - Координата x
 * @param sy - Координата y
 *
 * @return 0 - OK или 1 - ERROR
 */
bool duke_draw_from_file(const char *filename, size_t sx, size_t sy) {
    DukeHeader_t* meta = kmalloc(sizeof *meta);
    FILE* fp = fopen(filename, "r");

    if(!fp) {
        kfree(meta);
        return 1;
    }

	fread(fp, 1, sizeof(DukeHeader_t), meta);

    qemu_log("Draw Duke Image!");
    qemu_log("W: %d; H: %d; SIZE: %d; ALPHA: %s;", meta->width, meta->height, meta->data_length, meta->alpha ? "true" : "false");

    char *imagedata = kcalloc(meta->data_length, 1);
	fread(fp, meta->data_length, 1, imagedata);

    size_t x = 0, y = 0;
    char mod = meta->alpha ? 4 : 3;

    // qemu_log("Pixel step is: %d", mod);

    while(y < meta->height) {
        x = 0;
        while(x < meta->width) {
            size_t px = PIXIDX(meta->width * mod, x * mod, y);
            uint8_t r = imagedata[px];
            uint8_t g = imagedata[px + 1];
            uint8_t b = imagedata[px + 2];
            uint32_t color = RGB_TO_UINT32(r, g, b);
            
            if(meta->alpha) {
                uint8_t a = imagedata[px + 3];
                
                if(a != 0)
                    set_pixel(sx + x, sy + y, color);
            }else{
                set_pixel(sx + x, sy + y, color);
            }
            x++;
        }
        y++;
    }

    kfree(imagedata);
    fclose(fp);
    kfree(meta);
    qemu_log("Freed memory!!!");
    return 0;
}

void duke_rawdraw(const char *data, struct DukeImageMeta* meta, int sx, int sy){
    int x = 0, y = 0;
    char mod = meta->alpha?4:3;

    while(y < meta->height) {
        x = 0;
        while(x < meta->width) {
            int px = PIXIDX(meta->width * mod, x * mod, y);

            char r = data[px];
            char g = data[px+1];
            char b = data[px+2];
            char a = data[px+3];
            int color = ((r&0xff)<<16)|((g&0xff)<<8)|(b&0xff);
            
            if((mod==4 && a!=0) || mod==3) {
                set_pixel(sx+x, sy+y, color);
            }
            
            x++;
        }
        y++;
    }
}

void duke_rawdraw2(const char *data, int width, int height, int bpp, int sx, int sy){
	int x = 0, y = 0;

	while(y < height) {
		x = 0;
		while(x < width) {
			int px = PIXIDX(width * (bpp >> 3), x * (bpp >> 3), y);

			char r = data[px];
			char g = data[px+1];
			char b = data[px+2];
			char a = data[px+3];
			int color = ((r&0xff)<<16)|((g&0xff)<<8)|(b&0xff);

			if(((bpp >> 3) == 4 && a != 0) || (bpp >> 3) == 3) {
				set_pixel(sx+x, sy+y, color);
			}

			x++;
		}
		y++;
	}
}


void duke_scale(const char* pixels, unsigned int w1, unsigned int h1, int w2, int h2, char alpha, char* out) {
    int scr_w = (w1<<16)/w2;
    int scr_h = (h1<<16)/h2;

    int x = 0;
    int y = 0;

    int x2 = 0;
    int y2 = 0;

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

unsigned int duke_calculate_bufsize(unsigned int width, unsigned int height, unsigned int alpha) {
    return width*height*(alpha?4:3);
}


/*
DUKE SCALING EXAMPLE:

    struct DukeImageMeta* frw = duke_get_image_metadata("/initrd/res/SynapseOSLogo.duke");
    if(frw==0) {tty_printf("Return 0!\n");}

    unsigned int bsize = duke_calculate_bufsize(frw->width, frw->height, frw->alpha);

    char* imdata = kheap_malloc(bsize);
    duke_get_image_data("/initrd/res/SynapseOSLogo.duke", *frw, imdata);

    unsigned int nsize = duke_calculate_bufsize(300, 300, 1);
    char *ndata = kheap_malloc(nsize);

    duke_scale(imdata, frw->width, frw->height, 300, 300, 1, ndata);

    kheap_free(imdata);
    frw->width = 300;
    frw->height= 300;
    duke_rawdraw(ndata, frw, getScreenWidth()-300, 0);
    kheap_free(ndata);
*/

char duke_draw_scaled(char* filename, int width, int height, int x, int y) {
    if(!vfs_exists(filename)) return 1;
    struct DukeImageMeta origimg;
    
    char errcode = duke_get_image_metadata(filename, &origimg);
    if(errcode != 0) return 1;

    unsigned int bsize = duke_calculate_bufsize(origimg.width, origimg.height, origimg.alpha);

    char* imdata = kmalloc(bsize);
    duke_get_image_data(filename, origimg, imdata);

    unsigned int nsize = duke_calculate_bufsize(width, height, origimg.alpha);
    char *ndata = kmalloc(nsize);

    duke_scale(imdata, origimg.width, origimg.height, width, height, origimg.alpha, ndata);
    kfree(imdata);

    origimg.width  = width;
    origimg.height = height;
    
    duke_rawdraw(ndata, &origimg, x, y);
    
    kfree(ndata);
    return 0;
}
