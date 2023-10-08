/**
 * @file io/tty.c
 * @author Пиминов Никита (nikita.piminoff@yandex.ru)
 * @brief Средства для работы с видеодрайвером
 * @version 0.3.3
 * @date 2022-10-01
 * @copyright Copyright SayoriOS Team (c) 2022-2023
 */

#include <kernel.h>
#include <stdarg.h>
#include <sys/memory.h>
#include <io/tty.h>
#include <sys/scheduler.h>
#include <io/ports.h>
#include <io/status_loggers.h>
#include <drv/fpu.h>
#include <lib/math.h>

// TODO: Eurica! Split tty.c into 2 files:
//       tty.c - only text processing functions
//       default_console.c - TTY client

// TODO: Keep here.
volatile uint8_t tty_feedback = 1;		///< ...
size_t tty_line_fill[1024];				///< ....
uint32_t tty_pos_x = 0;						///< Позиция на экране по X
uint32_t tty_pos_y = 0;						///< Позиция на экране по Y
int32_t tty_off_pos_x = 8;					///< ...
int32_t tty_off_pos_p = 0;					///< ...
uint32_t tty_off_pos_h = 16;					///< ...
uint32_t tty_text_color;				///< Текущий цвет шрифта
uint32_t tty_bg_color;                  ///< Текущий задний фон
bool stateTTY = true;					///< Статус, разрешен ли вывод текста через tty_printf
/////////////////////////////////

// TODO: Move to things/cursor.c
thread_t* threadTTY01;					///< Поток с анимацией курсора
bool showAnimTextCursor = true;		///< Отображать ли анимацию курсора
void animTextCursor();
////////////////////////////////



/**
 * @brief Инициализация потоков
 */
void tty_taskInit(){
    process_t* proc = get_current_proc();
    threadTTY01 = thread_create(proc,
			   (void*)&animTextCursor,
			   0x4000,
			   true,
			   false);
}

/**
 * @brief Инициализация системы для печати через шрифты
 */
void tty_fontConfigurate(){
    qemu_log("[tFC] Configurate...");
    qemu_log("\t[TTY Configurator] Using default PSF fonts");
    tty_off_pos_x = 8;
    tty_off_pos_p = 0;
    tty_off_pos_h = 16;
	qemu_log("TTY_OFF_POS_X: %d; TTY_OFF_POS_P: %d; TTY_OFF_POS_H: %d", tty_off_pos_x, tty_off_pos_p, tty_off_pos_h);
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
 * @brief Получение позиции по x
 *
 * @return int32_t - Позиция по x
 */
uint32_t getPosX(){
    return tty_pos_x;
}


/**
 * @brief Получение позиции по y
 *
 * @return int32_t - Позиция по y
 */
uint32_t getPosY(){
    return tty_pos_y;
}

void set_cursor_enabled(bool en) {
    showAnimTextCursor = en;
}

/**
 * @brief Изменение цвета текста
 *
 * @param color - цвет
 */
void tty_setcolor(uint32_t color) {
    tty_text_color = color;
}

uint32_t tty_getcolor() {
    return tty_text_color;
}

/**
 * @brief Изменение цвета заднего фона
 *
 * @param color - цвет
 */
void tty_set_bgcolor(uint32_t color) {
    tty_bg_color = color;
}

/**
 * @brief Прокрутка экрана на num_rows строк
 *
 */
void tty_scroll(uint32_t num_rows) {
	uint32_t row_pos_offset = tty_off_pos_h * num_rows;

	uint8_t *addr = (uint8_t*)getFrameBufferAddr();
	uint32_t pitch = getDisplayPitch();

    uint8_t *read_ptr = addr + (row_pos_offset * pitch);
    uint8_t *write_ptr = addr;

    tty_pos_y -= row_pos_offset;

    uint32_t num_bytes = (pitch * VESA_HEIGHT) - (pitch * row_pos_offset);
    
    memcpy(write_ptr, read_ptr, num_bytes);

    // Очистка строк
    write_ptr = addr + num_bytes;
    memset(write_ptr, 0, pitch * row_pos_offset);
}

/**
 * @brief Изменяем позицию курсора по X
 *
 * @param x - позиция по X
 */
void setPosX(uint32_t x){
    tty_pos_x = x;
}


/**
 * @brief Изменяем позицию курсора по Y
 *
 * @param y - позиция по Y
 */
void setPosY(uint32_t y){
    tty_pos_y = y;
}


/**
 * @brief Вывод линии на экран // LINES? DID YOU MEAN RECTS?
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



void buffer_set_pixel4(uint8_t *buffer, size_t width, size_t height, size_t x, size_t y, size_t color) {
    if(x >= width || y >= height)
        return;
    
    size_t pixpos = PIXIDX(width * 4, x * 4, y);

    // qemu_log("X: %d Y: %d POS: %d", x, y, pixpos);

    buffer[pixpos + 0] = (uint8_t)color;
    buffer[pixpos + 1] = (uint8_t)(color >> 8);
    buffer[pixpos + 2] = (uint8_t)(color >> 16);
//    buffer[pixpos + 3] = (uint8_t)(color >> 24);
}

/**
 * @brief Вывод одного символа с учетом цвета фона и текста
 *
 * @param c - символ
 * @param txColor - цвет текста
 * @param bgColor - цвет фона
 */
void _tty_putchar_color(char c,char c1, uint32_t txColor, uint32_t bgColor) {

    if ((tty_pos_x + tty_off_pos_x) >= (int) VESA_WIDTH || c == '\n') {
        tty_line_fill[tty_pos_y] = tty_pos_x;
        tty_pos_x = 0;

        if ((tty_pos_y + tty_off_pos_h) >= (int) VESA_HEIGHT) {
            tty_scroll(1);
        } else {
            tty_pos_y += tty_off_pos_h;
        }
    } else {

        if ((tty_pos_y + tty_off_pos_h) >= (int) VESA_HEIGHT) {
            tty_scroll(1);
        }
        
        tty_setcolor(txColor);
        draw_vga_ch(c,c1, tty_pos_x, tty_pos_y, tty_text_color);
        
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
void _tty_putchar(char c, char c1) {
    if ((tty_pos_x + tty_off_pos_x) >= (int) VESA_WIDTH || c == '\n') {
        tty_line_fill[tty_pos_y] = tty_pos_x;
        tty_pos_x = 0;

        if ((tty_pos_y + tty_off_pos_h) >= (int) VESA_HEIGHT) {
            tty_scroll(1);
        }
        
        tty_pos_y += tty_off_pos_h;
    } else if (c == '\t') {
        tty_pos_x += 4 * tty_off_pos_h;
    } else if(c != '\n') {
        if (tty_pos_y + tty_off_pos_h >= (int) VESA_HEIGHT) {
            tty_scroll(1);
        }

        draw_vga_ch(c, c1, tty_pos_x, tty_pos_y, tty_text_color);
        
        tty_pos_x += tty_off_pos_x;
    }
}

void tty_putchar(char c, char c1) {
    _tty_putchar(c, c1);
    punch();
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
    // draw_vga_character(' ', tty_pos_x, tty_pos_y, tty_text_color, 0x000000, 1);
    // punch();
	drawRect(tty_pos_x, tty_pos_y, tty_off_pos_x, tty_off_pos_h, 0x000000);
    punch();
    // qemu_log("TTY BACKSPACE!!!");
}


/**
 * @brief Вывод строки
 *
 * @param str - строка
 */
void _tty_puts(const char str[]) {
    for (size_t i = 0, len = strlen(str); i < len; i++) {
        _tty_putchar(str[i], str[i+1]);
        if (isUTF(str[i])){
            i++;
        }
    }
}

/**
 * @brief Вывод цветного текста
 *
 * @param str - текст
 * @param txColor - цвет текста
 * @param bgColor - цвет фона
 */
void _tty_puts_color(const char str[], uint32_t txColor, uint32_t bgColor) {
    for (size_t i = 0, len = strlen(str); i < len; i++) {
        _tty_putchar_color(str[i],str[i+1], txColor, bgColor);
		if (isUTF(str[i])){
            i++;
        }
    }
}

void tty_puts_color(const char str[], uint32_t txColor, uint32_t bgColor) {
    // punch();
    _tty_puts_color(str, txColor, bgColor);
    punch();
}


/**
 * @brief Вывод целого числа
 *
 * @param i - число
 */
void _tty_putint(const ssize_t i) {
    char res[32];

    if (i < 0) {
        //tty_putchar('-');
    }

    itoa(i, res);
    _tty_puts(res);
}

void tty_putint(const ssize_t i) {
    _tty_putint(i);
    punch();
}

void _tty_putuint(const size_t i) {
    char res[32];

    itou(i, res);
    _tty_puts(res);
}

void tty_putuint(const size_t i) {
    _tty_putuint(i);
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
    // punch();
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
    // punch();
    _tty_puthex_v(i);
    punch();
}


/**
 * @brief Подфункция-обработчик для tty_printf
 *
 * @param format - строка форматов
 * @param args - аргументы
 */

#define NEW_TTY_PRINTF 0

#if NEW_TTY_PRINTF == 1
void _tty_print(const char* format, va_list args) {
	char* a;

	vasprintf(&a, format, args);

	_tty_puts(a);

	kfree(a);
}
#else
void _tty_print(const char *format, va_list args) {
    char* fmt = (char*)format;

    while (*fmt) {
        if (*fmt == '%') {
            size_t width = 0;
            bool left_align = false;
            
            fmt++;

            if(*fmt == '-') {
                left_align = true;
                fmt++;
            }

            while(isdigit(*fmt)) {
                width = width * 10 + (*fmt - '0');
                fmt++;
            }

            if(*fmt == '*') {
                width = va_arg(args, int);
                fmt++;
            }

            // qemu_log("Width is: %d", width);

            switch (*fmt) {
                case 's': {
                    char* arg = va_arg(args, char*);

                    int space = (int)width - (int)strlen(arg);

                    if(left_align)
                        _tty_puts(arg ? arg : "(nullptr)");

                    if(space > 0) {
                        while(space--)
                            _tty_puts(" ");
                    }

                    if(!left_align)
                        _tty_puts(arg ? arg : "(nullptr)");

                    break;
                }
                case 'c': {
                    _tty_putchar(va_arg(args, int), 0);
                    break;
                }
                case 'f': {
                    double a = va_arg(args, double);
					if(!fpu_isInitialized()) {
						_tty_puts("0.FPUNOINIT");
						break;
					}


					if(a < 0) {
						a = -a;
						
                        _tty_puts("-");
					}

                    if(a == NAN) {
                        _tty_puts("nan");
                        break;
                    }

                    if(a == INFINITY) {
                        _tty_puts("inf");
                        break;
                    }

					double rem = a - (int)a;
					_tty_putint((int)a);
					_tty_puts(".");
					
                    for(int n = 0; n < 7; n++) {
				        _tty_putint((int)(rem * ipow(10, n+1)) % 10);
				    }
                    
                    break;
                }
                case 'i':
                case 'd': {
                    int num = va_arg(args, int);
                    int space = width - digit_count(num);

                    if(num < 0)
                        space++;

                    if(left_align)
                        _tty_putint(num);
                    
                    if(space > 0) {
                        while(space--)
                            _tty_puts(" ");
                    }

                    if(!left_align)
                        _tty_putint(num);
                    
                    break;
                }
                case 'u': {
                    unsigned int num = va_arg(args, unsigned int);
                    int space = width - digit_count((int)num);

                    if(num < 0)
                        space++;

                    if(left_align)
                        _tty_putint(num);
                    
                    if(space > 0) {
                        while(space--)
                            _tty_puts(" ");
                    }

                    if(!left_align)
                        _tty_putint(num);
                    
                    break;
                }
                case 'x': {
                    int num = va_arg(args, int);
                    int space = width - hex_count(num) - 2;

                    if(left_align)
                        _tty_puthex(num);
                    
                    if(space > 0) {
                        while(space--)
                            _tty_puts(" ");
                    }

                    if(!left_align)
                        _tty_puthex(num);
                    
                    break;
                }
                case 'v': {
                    int num = va_arg(args, int);
                    int space = width - hex_count(num);

                    if(left_align)
                        _tty_puthex_v(num);
                    
                    if(space > 0) {
                        while(space--)
                            _tty_puts(" ");
                    }

                    if(!left_align)
                        _tty_puthex_v(num);
                    
                    break;
                }
                default:
                    _tty_putchar(*fmt, *(fmt+1));
            }
        // \t
        } else if (*fmt == 9) {
            tty_pos_x += 4 * tty_off_pos_h;
        } else {
            _tty_putchar(*fmt, *(fmt+1));
            if (isUTF(*fmt))
                fmt++;
        }
        fmt++;
    }
}

#endif

void _tty_printf(char *text, ...) {
    //qemu_log("\nState sAQ: %d",showAnimTextCursor);
	int sAT = (showAnimTextCursor?1:0);
    if (sAT == 1){
		showAnimTextCursor = false;
		//qemu_log("Disabled sAQ: %d | %d",sAT,showAnimTextCursor);
	}
    if (stateTTY){
        va_list args;
        va_start(args, text);
        _tty_print(text, args);
        va_end(args);
    }

	if (sAT == 1){
		showAnimTextCursor = true;
		//qemu_log("Enabled sAQ: %d | %d\n",sAT,showAnimTextCursor);
	}
}

/**
 * @brief Анимация курсора (для tty)
 */
void animTextCursor(){
    qemu_log("animTextCursor Work...");
    bool vis = false;
    int ox = 0, oy = 0;

    while(1) {
        if(!showAnimTextCursor)
			continue;

		ox = getPosX();
        oy = getPosY();

        if (!vis){
            drawRect(ox,oy+tty_off_pos_h-3,tty_off_pos_x,3,0x333333);
            vis = true;
        } else {
            drawRect(ox,oy+tty_off_pos_h-3,tty_off_pos_x,3,0x000000);
            vis = false;
        }

		punch();
        sleep_ms(250);
    }

	// So, it never quits
//    qemu_log("animTextCursor complete...");
//    thread_exit(threadTTY01);
}

void clean_tty_screen() {
    clean_screen();

    tty_pos_x = 0;
    tty_pos_y = tty_off_pos_h;
} 
