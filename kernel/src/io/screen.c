#include <kernel.h>
#include <common.h>

uint8_t *framebuffer_addr;				///< Точка монтирования
uint32_t framebuffer_pitch;				///< Частота обновления экрана
uint32_t framebuffer_bpp;				///< Глубина цвета экрана
uint32_t framebuffer_width;				///< Длина экрана
uint32_t framebuffer_height;			///< Высота экрана
uint32_t framebuffer_size;				///< Кол-во пикселей
uint8_t *back_framebuffer_addr = 0;			///< Позиция буфера экрана
bool lazyDraw = true;					///< Включен ли режим ленивой прорисовки
bool tty_oem_mode = false;				///< Режим работы
void* (*fb_copier)(void*, const void*, size_t);                   ///< Depends on framebuffer size

/**
 * @brief Обновить информацию на основном экране
 */
void punch() {
    fb_copier(framebuffer_addr, back_framebuffer_addr, framebuffer_size);
}

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

/**
 * @brief Получение размера буфера экрана
 *
 * @return uint32_t - Размер буфера экрана
 */
size_t getDisplaySize(){
    return framebuffer_size;
}

void create_back_framebuffer() {
    qemu_log("^---- 1. Allocating");
    back_framebuffer_addr = kmalloc(framebuffer_size);

    qemu_log("framebuffer_size = %d (%dK) (%dM)", framebuffer_size, framebuffer_size/1024, framebuffer_size/(1024*1024));
    qemu_log("back_framebuffer_addr = %x", back_framebuffer_addr);
    
    memset(back_framebuffer_addr, 0, framebuffer_size);
}

/**
 * @brief Инициализация графики
 *
 * @param mboot - информация полученная от загрузчика
 */
void init_vbe(multiboot_header_t *mboot) {
    svga_mode_info_t *svga_mode = (svga_mode_info_t*) mboot->vbe_mode_info;
    framebuffer_addr = (uint8_t*)svga_mode->physbase;
    framebuffer_pitch = svga_mode->pitch;
    framebuffer_bpp = svga_mode->bpp;
    framebuffer_width = svga_mode->screen_width;
    framebuffer_height = svga_mode->screen_height;
    framebuffer_size = framebuffer_height * framebuffer_pitch;

    fb_copier = (framebuffer_size % 4 == 0 ? memcpy4 : (framebuffer_size % 2 == 0 ? memcpy2 : memcpy));

    qemu_log("Booster is: %d",  (framebuffer_size % 4 == 0 ? 4 : (framebuffer_size % 2 == 0 ? 2 : 1)));

    qemu_log("[VBE] [Install] W:%d H:%d B:%d S:%d M:%x",framebuffer_width,framebuffer_height,framebuffer_bpp,framebuffer_size,framebuffer_addr);
    physaddr_t frame;
    virtual_addr_t virt;
    physaddr_t to;

    physaddr_t kdir = get_kernel_dir();
    size_t start_tk = getTicks();

    for (frame = (physaddr_t)framebuffer_addr,
    	 virt = (virtual_addr_t)framebuffer_addr,
         to = (physaddr_t)(framebuffer_addr + framebuffer_size);
         frame < to;
         frame += PAGE_SIZE,
         virt += PAGE_SIZE) {
            // map_pages(kdir, frame, virt, PAGE_SIZE, (PAGE_PRESENT | PAGE_WRITEABLE));

            // FIXME: WHy it doesn't run when setting size = 1? (Workaround is 2048)
            map_pages(kdir, frame, virt, 2048, (PAGE_PRESENT | PAGE_WRITEABLE));
    }

    qemu_log("Okay mapping! (took %d millis)", (getTicks() - start_tk)/(getFrequency()/1000));

    // map_pages(get_kernel_dir(), framebuffer_addr, framebuffer_addr,
    //           framebuffer_size / PAGE_SIZE, 0x07);
    qemu_log("Creating framebuffer");
    create_back_framebuffer();
    qemu_log("^---- OKAY");
}

/**
 * @brief Получить цвет на пикселе по X и Y
 *
 * @param int32_t x - X
 * @param int32_t y - Y
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

/**
 * @brief Вывод одного пикселя на экран
 *
 * @param x - позиция по x
 * @param y - позиция по y
 * @param color - цвет
 */
void set_pixel(int32_t x, int32_t y, uint32_t color) {
    if (x < 0 || y < 0 ||
        x >= (int) VESA_WIDTH ||
        y >= (int) VESA_HEIGHT) {
            return;
    }

    uint8_t* pixels = back_framebuffer_addr + (x * (framebuffer_bpp >> 3)) + y * framebuffer_pitch;

    pixels[0] = color & 255;
    pixels[1] = (color >> 8) & 255;
    pixels[2] = (color >> 16) & 255;
}

void rgba_blend(uint8_t result[4], uint8_t fg[4], uint8_t bg[4])
{
    uint32_t alpha = fg[3] + 1;
    uint32_t inv_alpha = 256 - fg[3];

    result[0] = (uint8_t)((alpha * fg[0] + inv_alpha * bg[0]) >> 8);
    result[1] = (uint8_t)((alpha * fg[1] + inv_alpha * bg[1]) >> 8);
    result[2] = (uint8_t)((alpha * fg[2] + inv_alpha * bg[2]) >> 8);
    result[3] = 0xff;
}

void setPixelAlpha(int x, int y, rgba_color color) {
    if (x < 0 || y < 0 ||
        x >= (int) VESA_WIDTH ||
        y >= (int) VESA_HEIGHT) {
        return;
    }

    unsigned where = x * (framebuffer_bpp / 8) + y * framebuffer_pitch;

    if (color.a != 255) {
        if (color.a != 0) {

            uint8_t bg[4] = {back_framebuffer_addr[where], back_framebuffer_addr[where + 1], back_framebuffer_addr[where + 2], 255};
            uint8_t fg[4] = {(uint8_t)color.b, (uint8_t)color.g, (uint8_t)color.r, (uint8_t)color.a};
            uint8_t res[4];

            rgba_blend(res, fg, bg);

            framebuffer_addr[where] = res[0];
            framebuffer_addr[where + 1] = res[1];
            framebuffer_addr[where + 2] = res[2];

            back_framebuffer_addr[where] = res[0];
            back_framebuffer_addr[where + 1] = res[1];
            back_framebuffer_addr[where + 2] = res[2];

        } else { // if absolutely transparent dont draw anything
            return;
        }
    } else { // if non transparent just draw rgb
        framebuffer_addr[where] = color.b & 255;
        framebuffer_addr[where + 1] = color.g & 255;
        framebuffer_addr[where + 2] = color.r & 255;

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
uint32_t getWidthScreen(){
    return framebuffer_width;
}


/**
 * @brief Получение ширины экрана
 *
 * @return uint32_t - ширина
 */
uint32_t getHeightScreen(){
    return framebuffer_height;
}

/**
 * @brief Очистка экрана
 *
 */
void clean_screen(){
    memset(back_framebuffer_addr, 0, framebuffer_size);  // Optimized variant

    punch();
}