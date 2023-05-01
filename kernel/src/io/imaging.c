/**
 * @file io/imaging.c
 * @author Drew >_ (pikachu_andrey@vk.com)
 * @brief Встраиваемая библиотека рисования изображений формата Duke
 * @version 0.3.2
 * @date 2022-11-01
 * @copyright Copyright SayoriOS Team (c) 2022-2023
 */

#include <kernel.h>
#include <lib/stdio.h>
#include <io/imaging.h>
#include <io/ports.h>

// int pixidx(int width, int x, int y) {
//     return width * y + x;
// }


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

        int wx = sx, wy = sy;
        char mod = meta->alpha?4:3;

        while(wy<(height-sy)) {
            wx = sx;
            while((wx-sx)<(width)) {
                int px = PIXIDX(meta->width*mod, wx*mod, wy);
                int r = imagedata[px];
                int g = imagedata[px+1];
                int b = imagedata[px+2];
                int a = imagedata[px+3];
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
char duke_draw_from_file(const char *filename, size_t sx, size_t sy) {
    struct DukeImageMeta* meta = kmalloc(sizeof *meta);

    FILE* fp = fopen(filename, "r");

    if(!fp) return 1;

    fread_c(fp, 1, sizeof(struct DukeImageMeta), meta);

    char *imagedata = kcalloc(meta->data_length, sizeof(char));
    fread_c(fp, meta->data_length, 1, imagedata);

    size_t x = 0, y = 0;
    char mod = meta->alpha?4:3;

    // qemu_log("Pixel step is: %d", mod);

    while(y < meta->height) {
        x = 0;
        while(x < meta->width) {
            size_t px = PIXIDX(meta->width*mod, x*mod, y);
            uint8_t r = imagedata[px];
            uint8_t g = imagedata[px+1];
            uint8_t b = imagedata[px+2];
            uint8_t a = imagedata[px+3];
            uint32_t color = RGB_TO_UINT32(r, g, b);
            if(meta->alpha) {
                if(a!=0) set_pixel(sx+x, sy+y, color);
            }else{
                set_pixel(sx+x, sy+y, color);
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

void duke_rawdraw(char* data, struct DukeImageMeta* meta, int sx, int sy){
    int x = 0, y = 0;
    char mod = meta->alpha?4:3;

    while(y < meta->height) {
        x = 0;
        while(x < meta->width) {
            int px = PIXIDX(meta->width * mod, x * mod, y);

            int r = data[px];
            int g = data[px+1];
            int b = data[px+2];
            int a = data[px+3];
            int color = ((r&0xff)<<16)|((g&0xff)<<8)|(b&0xff);
            
            if((mod==4 && a!=0) || mod==3) {
                set_pixel(sx+x, sy+y, color);
            }
            
            x++;
        }
        y++;
    }
}

void duke_scale(char* pixels, unsigned int w1, unsigned int h1, int w2, int h2, char alpha, char* out) {
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
    duke_rawdraw(ndata, frw, getWidthScreen()-300, 0);
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

    origimg.width = width;
    origimg.height= height;
    
    duke_rawdraw(ndata, &origimg, x, y);
    
    kfree(ndata);
    return 0;
}
