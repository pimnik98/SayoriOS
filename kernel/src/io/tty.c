/**
 * @file io/tty.c
 * @author Пиминов Никита (nikita.piminoff@yandex.ru)
 * @brief Средства для работы с видеодрайвером
 * @version 0.3.0
 * @date 2022-10-01
 * @copyright Copyright SayoriOS Team (c) 2022
 */
#define ENABLE_DOUBLE_BUFFERING 1		///< Включение двухбуферной обработки

#include <kernel.h>
#include <stdarg.h>
#include <sys/memory.h>
#include <io/tty.h>
#include <io/ports.h>
#include <io/colors.h>
uint8_t *framebuffer_addr;				///< Точка монтирования
uint32_t framebuffer_pitch;				///< ...
uint32_t framebuffer_bpp;				///< ...
uint32_t framebuffer_width;				///< Длина экрана
uint32_t framebuffer_height;			///< Высота экрана
uint32_t framebuffer_size;				///< Кол-во пикселей
uint8_t *back_framebuffer_addr;			///< Позиция буфера экрана
volatile uint8_t tty_feedback = 1;		///< ...
size_t tty_line_fill[1024];				///< ....
int32_t tty_pos_x;						///< Позиция на экране по X
int32_t tty_pos_y;						///< Позиция на экране по Y
int32_t tty_off_pos_x;					///< ...
int32_t tty_off_pos_p;					///< ...
int32_t tty_off_pos_h;					///< ...
bool tty_oem_mode = false;				///< Режим работы
uint32_t tty_text_color;				///< Текущий цвет шрифта
uint32_t tty_bg_color;                  ///< Текущий задний фон
bool stateTTY = true;					///< Статус, разрешен ли вывод текста через tty_printf
bool lazyDraw = true;					///< Включен ли режим ленивой прорисовки
thread_t* threadTTY01;					///< Поток с анимацией курсора
bool showAnimTextCursor = false;		///< Отображать ли анимацию курсора

/**
 * @brief Обновить информацию на основном экране
 */
void punch() {
    #if ENABLE_DOUBLE_BUFFERING==1
    memcpy(framebuffer_addr, back_framebuffer_addr, framebuffer_size);
    #endif
}

/**
 * @brief Анимация курсора (для tty)
 */
void animTextCursor(){
    qemu_log("animTextCursor Work...");
    bool vis = false;
    int ox=0,oy=0,lx=0,ly=0;
    while (1){
        ox = getPosX();
        oy = getPosY();
        if (!vis){
            drawRect(ox,oy,9,9,0x333333);
            punch();
            vis = true;
        } else {
            drawRect(ox,oy,9,9,0x000000);
            punch();
            vis = false;
        }
        sleep_ms(500);
    }
    qemu_log("animTextCursor complete...");
    thread_exit(threadTTY01);
}
/**
 * @brief Инициализация потоков
 */
void tty_taskInit(){
    process_t* proc = get_current_proc();
    threadTTY01 = thread_create(proc,
			   &animTextCursor,
			   0x4000,
			   true,
			   false);
}

/**
 * @brief Установка режима работы
 *
 * @param bool oem - Режим работы
 */
void tty_set_oem(bool oem) {
    if(oem) {
        tty_off_pos_x = 8;
        tty_off_pos_p = 0;
        tty_off_pos_h = 17;
    }else{
        tty_off_pos_x = getConfigFonts(0);
        tty_off_pos_p = getConfigFonts(1);
        tty_off_pos_h = getConfigFonts(2);
    }
    tty_oem_mode = oem;
}

/**
 * @brief Инициализация системы для печати через шрифты
 */
void tty_fontConfigurate(){
    qemu_log("[tFC] Configurate...");
    qemu_log("\t * 0 -> %d",getConfigFonts(0));
    qemu_log("\t * 1 -> %d",getConfigFonts(1));
    qemu_log("\t * 2 -> %d",getConfigFonts(2));
    qemu_log("\t * 3 -> %d",getConfigFonts(3));

    if (getConfigFonts(3) != 0){
        qemu_log("\t[tFC] Sorry, but the font system isn't ready to go yet. Preset to work in OEM mode.");
        tty_off_pos_x = 8;
        tty_off_pos_p = 0;
        tty_off_pos_h = 17;
        tty_oem_mode = true;
    }
    tty_off_pos_x = getConfigFonts(0);
    tty_off_pos_p = getConfigFonts(1);
    tty_off_pos_h = getConfigFonts(2);
    tty_oem_mode = false;
}

/**
 * @brief Меняет состояние печати через printf
 *
 * @param bool state - Включить или выключить печать
 */
void tty_changeState(bool state){
    stateTTY = state;
}

/**
 * @brief Слияние символа и цвета, для вывода
 *
 */
uint16_t vga_entry(uint8_t c, uint8_t tty_color) {
    return (uint16_t) c | (uint16_t) tty_color << 8;
}


/**
 * @brief Получение позиции по x
 *
 * @return int32_t - позиция по x
 */
int32_t getPosX(){
    return tty_pos_x;
}


/**
 * @brief Получение позиции по y
 *
 * @return int32_t - позиция по y
 */
int32_t getPosY(){
    return tty_pos_y;
}


/**
 * @brief Изменение цвета текста
 *
 * @param color - цвет
 */
void tty_setcolor(int32_t color) {
    //qemu_log("[TTY] [setColor] %x",color);
    if (!tty_oem_mode){
        setColorFont(color);
    }
    tty_text_color = color;
}

/**
 * @brief Изменение цвета заднего фона
 *
 * @param color - цвет
 */
void tty_set_bgcolor(int32_t color) {
    if (!tty_oem_mode){
        setColorFontBg(color);
    }
    tty_bg_color = color;
}

/**
 * @brief Инициализация графики
 *
 * @param mboot - информация полученная от загрузчика
 */
void init_vbe(multiboot_header_t *mboot) {
    svga_mode_info_t *svga_mode = (svga_mode_info_t*) mboot->vbe_mode_info;
    framebuffer_addr = (uint8_t *)svga_mode->physbase;
    framebuffer_pitch = svga_mode->pitch;
    framebuffer_bpp = svga_mode->bpp;
    framebuffer_width = svga_mode->screen_width;
    framebuffer_height = svga_mode->screen_height;
    framebuffer_size = framebuffer_height * framebuffer_pitch;

    qemu_log("[VBE] [Install] W:%d H:%d B:%d S:%d M:%x",framebuffer_width,framebuffer_height,framebuffer_bpp,framebuffer_size,framebuffer_addr);
    physaddr_t frame;
    virtual_addr_t virt;

    for (frame = (physaddr_t)framebuffer_addr, virt = (virtual_addr_t)framebuffer_addr;
        frame < (physaddr_t)(framebuffer_addr + framebuffer_size);
        frame += PAGE_SIZE, virt += PAGE_SIZE) {
            //qemu_log("frame: %x | virt:%x",frame,virt);
            map_pages(get_kernel_dir(),frame,virt,PAGE_SIZE,0x07);
        //vmm_map_page(frame, virt);
    }
   qemu_log("VBE create_back_framebuffer");

   create_back_framebuffer(); // PAGE FAULT CAUSES HERE!!!
   qemu_log("^---- OKAY");
}


/**
 * @brief Создание второго буффера экрана
 *
 */
void create_back_framebuffer() {
    //qemu_log("Back FrameBuffer DISABLED!!!");   // Отключено, так как из-за этого получаем инъекцию
    //return;
	qemu_log("^---- 1. Allcoating"); // Я не знаю почему, но это предотвратило падение, но устроило его в другом месте
    back_framebuffer_addr = kmalloc(framebuffer_size);
    uint8_t* backfb = kmalloc(framebuffer_size);
    qemu_log("^---- 1. Allcoating %d",backfb);
    qemu_log("framebuffer_size = %d", framebuffer_size);

    qemu_log("back_framebuffer_addr = %x", back_framebuffer_addr);
    memset(backfb, 0, framebuffer_size); // Должно предотвратить падение

    memcpy(backfb, framebuffer_addr, framebuffer_size);

    back_framebuffer_addr = backfb;
}


/**
 * @brief Очистка экрана и сброс настроек
 *
 * @param mboot_info - информация о дисплее от загрузчика
 */
void tty_init(struct multiboot_header *mboot_info) {
    tty_pos_y = 0;
    tty_pos_x = 0;

    tty_text_color = COLOR_SYS_TEXT;

    svga_mode_info_t *svga_mode = (svga_mode_info_t*) mboot_info->vbe_mode_info;

    framebuffer_addr = (uint8_t *)svga_mode->physbase;
    framebuffer_pitch = svga_mode->pitch;
    framebuffer_bpp = svga_mode->bpp;
    framebuffer_width = svga_mode->screen_width;
    framebuffer_height = svga_mode->screen_height;
    framebuffer_size = framebuffer_height * framebuffer_pitch;
    back_framebuffer_addr = framebuffer_addr;
    tty_fontConfigurate();

}


/**
 * @brief Прокрутка экрана на 1 строку
 *
 */
void tty_scroll() {
    uint32_t num_rows = 1;
    tty_pos_y -= tty_off_pos_h*num_rows;

    // Копируем строки сверху
    uint8_t *read_ptr = (uint8_t*) back_framebuffer_addr + ((num_rows * tty_off_pos_h) * framebuffer_pitch);
    uint8_t *write_ptr = (uint8_t*) back_framebuffer_addr;
    uint32_t num_bytes = (framebuffer_pitch * VESA_HEIGHT) - (framebuffer_pitch * (num_rows * tty_off_pos_h));
    memcpy(write_ptr, read_ptr, num_bytes);

    // Очистка строк
    write_ptr = (uint8_t*) back_framebuffer_addr + (framebuffer_pitch * VESA_HEIGHT) - (framebuffer_pitch * (num_rows * tty_off_pos_h));
    memset(write_ptr, 0, framebuffer_pitch * (num_rows * tty_off_pos_h));

    // Копируем буфферы
    #if ENABLE_DOUBLE_BUFFERING==0
    memcpy(framebuffer_addr, back_framebuffer_addr, framebuffer_size);
    #endif
}

/**
 * @brief Получить цвет на пикселе по X и Y
 *
 * @param int32_t x - X
 * @param int32_t y - Y
 *
 * @return uint32_t - Цвет
 */
uint32_t getPixel(int32_t x, int32_t y){
    if (x < 0 || y < 0 ||
        x >= (int) VESA_WIDTH ||
        y >= (int) VESA_HEIGHT) {
        return 0x000000;
    }
    unsigned where = x * (framebuffer_bpp / 8) + y * framebuffer_pitch;
    #if ENABLE_DOUBLE_BUFFERING==0
    return ((framebuffer_addr[where+2] & 0xff) << 16) + ((framebuffer_addr[where+1] & 0xff) << 8) + (framebuffer_addr[where] & 0xff);
    #else
    //back_framebuffer_addr
    return ((back_framebuffer_addr[where+2] & 0xff) << 16) + ((back_framebuffer_addr[where+1] & 0xff) << 8) + (back_framebuffer_addr[where] & 0xff);
    #endif
}

/**
 * @brief Вывод одного пикселя на экран
 *
 * @param x - позиция по x
 * @param y - позиция по y
 * @param color - цвет
 */
void set_pixel(int32_t x, int32_t y, uint32_t color) {
    //qemu_log("[SET] X: %d Y: %d Color: %x",x,y,color);
    if (x < 0 || y < 0 ||
        x >= (int) VESA_WIDTH ||
        y >= (int) VESA_HEIGHT) {
            //qemu_log("\t[SET] ERR");
            return;
    }

    if (lazyDraw){
        if (getPixel(x,y) == color){
            //countLazySkip++;
            //qemu_log("[lazyDraw] Skipping paint...");
            return;
        }
        // Ленивая отрисовка
    }

    unsigned where = x * (framebuffer_bpp / 8) + y * framebuffer_pitch;

    //qemu_log("\t[SET] WHERE: %x | %d ",where,where);
    #if ENABLE_DOUBLE_BUFFERING==0
    framebuffer_addr[where] = color;
    framebuffer_addr[where + 1] = color >> 8;
    framebuffer_addr[where + 2] = color >> 16;
    #else
    back_framebuffer_addr[where] = color & 255;
    back_framebuffer_addr[where + 1] = (color >> 8) & 255;
    back_framebuffer_addr[where + 2] = (color >> 16) & 255;
    #endif
}


/**
 * @brief Изменяем позицию курсора по X
 *
 * @param x - позиция по X
 */
void setPosX(int32_t x){
    tty_pos_x = x;
}


/**
 * @brief Изменяем позицию курсора по Y
 *
 * @param y - позиция по Y
 */
void setPosY(int32_t y){
    tty_pos_y = y;
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
    for (int32_t x = 0; x < VESA_WIDTH; x++){
        for (int32_t y = 0; y < VESA_HEIGHT; y++){
            set_pixel(x, y, VESA_BLACK);
        }
    }

    tty_pos_x = 0;
    tty_pos_y = -tty_off_pos_h;
    punch();
    //qemu_log("Screan cleaned!");
}


/**
 * @brief Вывод линии на экран
 *
 * @param x - координата точки 1 по x
 * @param y - координата точки 1 по y
 * @param xe - координата точки 2 по x
 * @param ye - координата точки 2 по y
 * @param color - цвет
 */
void set_line(int32_t x, int32_t y, int32_t xe, int32_t ye, uint32_t color){
    for (int32_t i = x; i < xe; i++) {
        for (int32_t j = y; j < ye; j++) {
            set_pixel(i, j, color);
        }
    }
}

/**
 * @brief Рисуем прямоугольник
 *
 * @param x - Начальная координата X
 * @param y - Начальная координата y
 * @param w - Длина
 * @param h - Высота
 * @param color - цвет заливки
 */
void drawRect(int x,int y,int w, int h,int color){
    for (int _x = x; _x < x+w ; _x++){
        for (int _y = y; _y < y+h; _y++){
            set_pixel(_x, _y, color);
        }
    }
}


/**
 * @brief Вывод одного символа с учетом цвета фона и текста
 *
 * @param c - символ
 * @param txColor - цвет текста
 * @param bgColor - цвет фона
 */
void _tty_putchar_color(char c,char c1, uint32_t txColor, uint32_t bgColor) {

    if ((tty_pos_x + tty_off_pos_x) >= (int)VESA_WIDTH || c == '\n') {
        tty_line_fill[tty_pos_y] = tty_pos_x;
        tty_pos_x = 0;

        if ((tty_pos_y + tty_off_pos_h) >= (int)VESA_HEIGHT) {
            tty_scroll();
        } else {
            tty_pos_y += tty_off_pos_h;
        }
    } else {

        if ((tty_pos_y + tty_off_pos_h) >= (int)VESA_HEIGHT) {
            tty_scroll();
        }
        if (!tty_oem_mode){
            drawCharFont(c,c1,tty_pos_x, tty_pos_y,0);
            tty_setcolor(txColor);
        } else {
            draw_vga_character(c, tty_pos_x, tty_pos_y, txColor, bgColor, 1);
        }
        tty_pos_x += tty_off_pos_x;
    }
}

void tty_putchar_color(char c,char c1, uint32_t txColor, uint32_t bgColor) {
    _tty_putchar_color(c,c1, txColor, bgColor);
    punch();
}

/**
 * @brief Вывод одного символа
 *
 * @param c - символ
 */
void _tty_putchar(char c,char c1) {
    if ((tty_pos_x + tty_off_pos_x) >= (int)VESA_WIDTH || c == '\n') {
        tty_line_fill[tty_pos_y] = tty_pos_x;
        tty_pos_x = 0;

        if ((tty_pos_y + tty_off_pos_h) >= (int)VESA_HEIGHT) {
            tty_scroll();
        } else {
            tty_pos_y += tty_off_pos_h;
        }
    } else {
        if ((tty_pos_y + tty_off_pos_h) >= (int)VESA_HEIGHT) {
            tty_scroll();
        }
        if (!tty_oem_mode){
            drawCharFont(c,c1,tty_pos_x, tty_pos_y,0);
            //tty_setcolor(tty_text_color);
        } else {
            draw_vga_character(c, tty_pos_x, tty_pos_y, tty_text_color, 0x000000, 0);
        }
        tty_pos_x += tty_off_pos_x;
    }
}

void tty_putchar(char c,char c1) {
    _tty_putchar(c,c1);
    punch();
}


/**
 * @brief Вывод символа на экран с учетом позиции, цвета текста и фона
 *
 * @param c - символ
 * @param x - позиция по x
 * @param y - позиция по y
 * @param fg - цвет текста
 * @param bg - цвет фона
 * @param bgon - поменять ли местами цвета
 */
void draw_vga_character(uint8_t c, int32_t x, int32_t y, int32_t fg, int32_t bg, bool bgon) {

}


/**
 * @brief Удаление последнего символа
 *
 */
void tty_backspace() {
    if (tty_pos_x < tty_off_pos_x) { // Old: == 0
        if (tty_pos_y >= tty_off_pos_h) {
            tty_pos_y -= tty_off_pos_h;
        }
        tty_pos_x = tty_line_fill[tty_pos_y];
    } else {
        tty_pos_x -= tty_off_pos_x;
    }
    draw_vga_character(' ', tty_pos_x, tty_pos_y, tty_text_color, 0x000000, 1);
    punch();
    qemu_log("TTY BACKSPACE!!!");
}


/**
 * @brief Вывод строки
 *
 * @param str - строка
 */
void _tty_puts(const char str[]) {
    for (size_t i = 0; i < strlen(str); i++) {
        _tty_putchar(str[i],str[i+1]);
        if (isUTF(str[i])){
            i++;
        }
    }
}

void tty_puts(const char str[]) {
    _tty_puts(str);
    punch();
}


/**
 * @brief Вывод цветного текста
 *
 * @param str - текст
 * @param txColor - цвет текста
 * @param bgColor - цвет фона
 */
void _tty_puts_color(const char str[], uint32_t txColor, uint32_t bgColor) {
    for (size_t i = 0; i < strlen(str); i++) {
        _tty_putchar_color(str[i],str[i+1], txColor, bgColor);
    }
}

void tty_puts_color(const char str[], uint32_t txColor, uint32_t bgColor) {
    _tty_puts_color(str, txColor, bgColor);
    punch();
}


/**
 * @brief Вывод целого числа
 *
 * @param i - число
 */
void _tty_putint(const int32_t i) {
    char res[32];

    if (i < 0) {
        //tty_putchar('-');
    }

    itoa(i, res);
    _tty_puts(res);
}

void tty_putint(const int32_t i) {
    _tty_putint(i);
    punch();
}

/**
 * @brief Вывод HEX числа
 *
 * @param i - число
 */
void _tty_puthex(uint32_t i) {
    const unsigned char hex[16]  =  { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
    uint32_t n, d = 0x10000000;

    _tty_puts("0x");

    while((i / d == 0) && (d >= 0x10)) d /= 0x10;

    n = i;

    while( d >= 0xF ) {
        _tty_putchar(hex[n/d],0);
        n = n % d;
        d /= 0x10;
    }

    _tty_putchar(hex[n],0);
}

void tty_puthex(uint32_t i) {
    _tty_puthex(i);
    punch();
}


/**
 * @brief Вывод числа HEX без префикса 0x
 *
 * @param i - число
 */
void _tty_puthex_v(uint32_t i) {
    const unsigned char hex[16]  =  { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
    uint32_t n, d = 0x10000000;

    while((i / d == 0) && (d >= 0x10)) d /= 0x10;

    n = i;

    while( d >= 0xF ) {
        _tty_putchar(hex[n/d],0);
        n = n % d;
        d /= 0x10;
    }

    _tty_putchar(hex[n],0);
}

void tty_puthex_v(uint32_t i) {
    _tty_puthex_v(i);
    punch();
}


/**
 * @brief Подфункция-обработчик для tty_printf
 *
 * @param format - строка форматов
 * @param args - аргументы
 */
void _tty_print(char *format, va_list args) {
    int32_t i = 0;

    while (format[i]) {
        if (format[i] == '%') {
            i++;
            switch (format[i]) {
                case 's':
                    _tty_puts(va_arg(args, char*));
                    break;
                case 'c':
                    _tty_putchar(va_arg(args, int),0);
                    break;
                case 'f':
                    _tty_putchar(va_arg(args, float),0);
                    break;
                case 'd':
                    _tty_putint(va_arg(args, int));
                    break;
                case 'i':
                    _tty_putint(va_arg(args, int));
                    break;
                case 'u':
                    _tty_putint(va_arg(args, unsigned int));
                    break;
                case 'x':
                    _tty_puthex(va_arg(args, uint32_t));
                    break;
                case 'v':
                    _tty_puthex_v(va_arg(args, uint32_t));
                    break;
                default:
                    _tty_putchar(format[i],format[i+1]);
            }
            // \n
        } else if (format[i] == 10) {
            tty_line_fill[tty_pos_y] = tty_pos_x;
            tty_pos_x = 0;

            if ((tty_pos_y + tty_off_pos_h) >= (int)VESA_HEIGHT) {
                tty_scroll();
            } else {
                tty_pos_y += tty_off_pos_h;
            }
            // \t
        } else if (format[i] == 9) {
            tty_pos_x += 4 * tty_off_pos_h;
        } else {
            _tty_putchar(format[i],format[i+1]);
            if (isUTF(format[i])){
                i++;
            }
        }
        i++;
    }
}

void tty_print(char *format, va_list args) {
    _tty_print(format, args);
    punch();
}


/**
 * @brief Вывод форматированной строки на экран (аналог printf)
 *
 * @param text - строка форматов
 * @param ... - параметры
 */
void tty_printf(char *text, ...) {
    if (stateTTY){
        va_list args;
        va_start(args, text);
        tty_print(text, args);
        va_end(args);
    }
}
