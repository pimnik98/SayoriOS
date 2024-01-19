/**
 * @file io/tty.c
 * @author Пиминов Никита (nikita.piminoff@yandex.ru)
 * @brief Средства для работы с видеодрайвером
 * @version 0.3.5
 * @date 2022-10-01
 * @copyright Copyright SayoriOS Team (c) 2022-2024
 */

#include <stdarg.h>
#include <mem/vmm.h>
#include <io/tty.h>
#include <sys/scheduler.h>
#include <io/ports.h>
#include <io/status_loggers.h>
#include <drv/fpu.h>
#include <lib/math.h>
#include <io/rgb_image.h>
#include "lib/sprintf.h"
#include "drv/psf.h"

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
    qemu_log("Starting task...");
    process_t* proc = get_current_proc();
    qemu_log("Process at: %x", proc);
    threadTTY01 = thread_create(proc,
			   &animTextCursor,
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
 * @param state - Включить или выключить печать
 */
void tty_changeState(bool state){
    stateTTY = state;
}

/**
 * @brief Получение позиции по x
 *
 * @return Позиция по x
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
 * @brief Устновливает пиксель RGB в буфере в котором все пиксели представляют собой RGBA (альфа канал игнорируется)
 * @param buffer - буфер RGBA
 * @param width - длина кадра который представляет буфер
 * @param height - ширина кадра который представляет буфер
 * @param x - координата x
 * @param y - координата у
 * @param color - цвет в формате RGB
 */
void buffer_set_pixel4(uint8_t *buffer, size_t width, size_t height, size_t x, size_t y, size_t color) {
    if(x >= width || y >= height)
        return;
    
    size_t pixpos = PIXIDX(width * 4, x * 4, y);

    buffer[pixpos + 0] = (uint8_t)color;
    buffer[pixpos + 1] = (uint8_t)(color >> 8);
    buffer[pixpos + 2] = (uint8_t)(color >> 16);
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

	drawRect(tty_pos_x, tty_pos_y, tty_off_pos_x, tty_off_pos_h, 0x000000);
    punch();
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
 * @brief Подфункция-обработчик для tty_printf
 *
 * @param format - строка форматов
 * @param args - аргументы
 */
void _tty_print(const char* format, va_list args) {
	char* a = 0;

	vasprintf(&a, format, args);

	_tty_puts(a);

	kfree(a);
}

void _tty_printf(char *text, ...) {
	int sAT = (showAnimTextCursor?1:0);
    if (sAT == 1){
		showAnimTextCursor = false;
	}
    if (stateTTY){
        va_list args;
        va_start(args, text);
        _tty_print(text, args);
        va_end(args);
    }

	if (sAT == 1){
		showAnimTextCursor = true;
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

void clean_tty_screen_no_update() {
	clean_screen();

	tty_pos_x = 0;
	tty_pos_y = 0;
}

void clean_tty_screen() {
    clean_tty_screen_no_update();

	punch();
} 
