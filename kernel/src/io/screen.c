#include <io/screen.h>
#include <multiboot.h>
#include <lib/stdlib.h>
#include <sys/timer.h>
#include <io/ports.h>
#include <common.h>
#include "mem/pmm.h"
#include "mem/vmm.h"

uint8_t *framebuffer_addr = 0;				///< Точка монтирования
uint32_t framebuffer_pitch;				///< Частота обновления экрана
uint32_t framebuffer_bpp;				///< Глубина цвета экрана
uint32_t framebuffer_width;				///< Длина экрана
uint32_t framebuffer_height;			///< Высота экрана
uint32_t framebuffer_size;				///< Кол-во пикселей
uint8_t *back_framebuffer_addr = 0;		///< Позиция буфера экрана
bool lazyDraw = true;					///< Включен ли режим ленивой прорисовки
bool tty_oem_mode = false;				///< Режим работы

/**
 * @brief Получение адреса расположения драйвера экрана
 *
 * @return size_t - Адрес расположения
 */
size_t getDisplayAddr(){
    return (size_t)framebuffer_addr;
}

size_t getFrameBufferAddr() {
    return (size_t)back_framebuffer_addr;
}

/**
 * @brief Получение частоты обновления экрана
 *
 * @return uint32_t - Частота обновления
 */
uint32_t getDisplayPitch(){
    return framebuffer_pitch;
}

/**
 * @brief Получение глубины цвета экрана
 *
 * @return uint32_t - Глубина цвета
 */
uint32_t getDisplayBpp(){
    return framebuffer_bpp;
}

void create_back_framebuffer() {
    qemu_log("^---- 1. Allocating");
    back_framebuffer_addr = (uint8_t*)kcalloc(framebuffer_size, 1);

    qemu_log("framebuffer_size = %d (%dK) (%dM)", framebuffer_size, framebuffer_size/1024, framebuffer_size/(1024*1024));
    qemu_log("back_framebuffer_addr = %x", back_framebuffer_addr);
}

/**
 * @brief Инициализация графики
 *
 * @param mboot - информация полученная от загрузчика
 */
void init_vbe(multiboot_header_t *mboot) {
    // FIXME: Something wrong with `svga_mode_info_t` structure!
//    svga_mode_info_t *svga_mode = (svga_mode_info_t*) mboot->vbe_mode_info;
//    framebuffer_addr = (uint8_t*)svga_mode->physbase;
//    framebuffer_pitch = svga_mode->pitch;
//    framebuffer_bpp = svga_mode->bpp;
//    framebuffer_width = svga_mode->screen_width;
//    framebuffer_height = svga_mode->screen_height;
//    framebuffer_size = framebuffer_height * framebuffer_pitch;

//    qemu_log("[VBE] [Install] Width: %d; Height: %d; Pitch: %d; BPP: %d; Size: %d; Address: %x",
//             framebuffer_width,
//             framebuffer_height,
//             framebuffer_pitch,
//             framebuffer_bpp,
//             framebuffer_size,
//             framebuffer_addr
//    );


    framebuffer_addr = (uint8_t *) mboot->framebuffer_addr;
    framebuffer_pitch = mboot->framebuffer_pitch;
    framebuffer_bpp = mboot->framebuffer_bpp;
    framebuffer_width = mboot->framebuffer_width;
    framebuffer_height = mboot->framebuffer_height;
    framebuffer_size = framebuffer_height * framebuffer_pitch;

    qemu_log("[VBE] [USING LEGACY INFO] Width: %d; Height: %d; Pitch: %d; BPP: %d; Size: %d; Address: %x",
             mboot->framebuffer_width,
             mboot->framebuffer_height,
             mboot->framebuffer_pitch,
             mboot->framebuffer_bpp,
             mboot->framebuffer_height * mboot->framebuffer_pitch,
             mboot->framebuffer_addr
    );
    
    physical_addr_t frame = (physical_addr_t)framebuffer_addr;
    virtual_addr_t virt = (virtual_addr_t)framebuffer_addr;

    size_t start_tk = getTicks();

	map_pages(get_kernel_page_directory(),
			  frame,
			  virt,
			  framebuffer_size,
			  PAGE_WRITEABLE);

    qemu_log("Okay mapping! (took %d millis)", (getTicks() - start_tk)/(getFrequency()/1000));

    qemu_log("Creating framebuffer");
    create_back_framebuffer();
    qemu_log("^---- OKAY");
}

/**
 * @brief Получить цвет на пикселе по X и Y
 *
 * @param x - X
 * @param y - Y
 *
 * @return uint32_t - Цвет
 */
size_t getPixel(int32_t x, int32_t y){
    if (x < 0 || y < 0 ||
			x >= (int) VESA_WIDTH ||
        y >= (int) VESA_HEIGHT) {
        return 0x000000;
    }

    size_t where = x * (framebuffer_bpp >> 3) + y * framebuffer_pitch;

    return ((back_framebuffer_addr[where+2] & 0xff) << 16) + ((back_framebuffer_addr[where+1] & 0xff) << 8) + (back_framebuffer_addr[where] & 0xff);
}

void rgba_blend(uint8_t result[4], const uint8_t fg[4], const uint8_t bg[4])
{
    uint32_t alpha = fg[3] + 1;
    uint32_t inv_alpha = 256 - fg[3];

    result[0] = (uint8_t)((alpha * fg[0] + inv_alpha * bg[0]) >> 8);
    result[1] = (uint8_t)((alpha * fg[1] + inv_alpha * bg[1]) >> 8);
    result[2] = (uint8_t)((alpha * fg[2] + inv_alpha * bg[2]) >> 8);
    result[3] = 0xff;
}

void setPixelAlpha(uint32_t x, uint32_t y, rgba_color color) {
    if (x >= VESA_WIDTH ||
        y >= VESA_HEIGHT) {
        return;
    }

    unsigned where = x * (framebuffer_bpp / 8) + y * framebuffer_pitch;

    if (color.a != 255) {
        if (color.a != 0) {
            uint8_t bg[4] = {back_framebuffer_addr[where], back_framebuffer_addr[where + 1], back_framebuffer_addr[where + 2], 255};
            uint8_t fg[4] = {(uint8_t)color.b, (uint8_t)color.g, (uint8_t)color.r, (uint8_t)color.a};
            uint8_t res[4];

            rgba_blend(res, fg, bg);

            // framebuffer_addr[where] = res[0];
            // framebuffer_addr[where + 1] = res[1];
            // framebuffer_addr[where + 2] = res[2];

            back_framebuffer_addr[where] = res[0];
            back_framebuffer_addr[where + 1] = res[1];
            back_framebuffer_addr[where + 2] = res[2];

        } else { // if absolutely transparent don't draw anything
            return;
        }
    } else { // if non transparent just draw rgb
        // framebuffer_addr[where] = color.b & 255;
        // framebuffer_addr[where + 1] = color.g & 255;
        // framebuffer_addr[where + 2] = color.r & 255;

        back_framebuffer_addr[where] = color.b & 255;
        back_framebuffer_addr[where + 1] = color.g & 255;
        back_framebuffer_addr[where + 2] = color.r & 255;
    }
}

/**
 * @brief Получение длины экрана
 *
 * @return uint32_t - длина
 */
uint32_t getScreenWidth(){
    return framebuffer_width;
}


/**
 * @brief Получение ширины экрана
 *
 * @return uint32_t - ширина
 */
uint32_t getScreenHeight(){
    return framebuffer_height;
}

/**
 * @brief Очистка экрана
 *
 */
void clean_screen(){
    memset(back_framebuffer_addr, 0, framebuffer_size);  // Optimized variant
}

void rect_copy(int x, int y, int width, int height) {
    unsigned char* src = (unsigned char*)back_framebuffer_addr + (y * framebuffer_pitch) + (x * (framebuffer_bpp/8));
    unsigned char* dest = (unsigned char*)framebuffer_addr + (y * framebuffer_pitch) + (x * (framebuffer_bpp/8));
    size_t bytes_per_line = width * (framebuffer_bpp/8);
    
    for(int i = 0; i < height; i++) {
        memcpy(dest, src, bytes_per_line);
        src += framebuffer_pitch;
        dest += framebuffer_pitch;
    }
}
