/**
 * @file lib/pixel.c
 * @author Пиминов Никита (nikita.piminoff@yandex.ru)
 * @brief Драйвера для работы с пикселями (Пакет Пиксель)
 * @version 0.3.3
 * @date 2022-10-20
 * @copyright Copyright SayoriOS Team (c) 2022-2023
 */
#include <kernel.h>
#include <io/ports.h>
#include <io/duke_image.h>
#include <io/status_loggers.h>
#include <lib/stdio.h>
#include <lib/tui.h>

/**
 * @brief Рисует линию (На 16 пикселей)
 *
 * @param y - Начальная координата y
 * @param color - Цвет линии
*/
void drawLine(int y,int color){
    drawRect(0, y, getScreenWidth(), y * 16, color);
}

/**
 * @brief Рисуем залитый прямоугольник
 *
 * @param x - Начальная координата X
 * @param y - Начальная координата y
 * @param w - Длина
 * @param h - Высота
 * @param color - цвет заливки
 */
void drawRect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t color){
	for (__typeof__(x) _y = y; _y < y+h; _y++){
		for (__typeof__(x) _x = x; _x < x+w ; _x++){
            set_pixel(_x, _y, color);
        }
    }
}

/**
 * @brief Рисует узор вокруг прямоугольника
 *
 * @param x - Начальная координата X
 * @param y - Начальная координата y
 * @param w - Длина
 * @param h - Высота
 * @param color - Цвет 1
 * @param color2 - Цвет 2
 * @param с - Символ
*/
void drawRectLine(int x,int y,int w, int h,int color,int color2, int c){
	/**
    for (int _x = x; _x < x+w ; _x += 8){
        tty_putchar_color(c, _x, y, color, color2);
        tty_putchar_color(c, _x, y+h-16, color, color2);
    }
    for (int _y = y; _y < y+h; _y += 16){
        tty_putchar_color(c, x, _y, color, color2);
        tty_putchar_color(c, x+w-8, _y, color, color2);
    }
	*/
} 

/**
 * @brief Рисует линию вокруг прямоугольника
 *
 * @param x - Начальная координата X
 * @param y - Начальная координата y
 * @param w - Длина
 * @param h - Высота
 * @param color - Цвет
*/
void drawRectBorder(int x, int y, int w, int h, int color){
	for (int _x = x; _x < x+w ; _x++){
		set_pixel(_x, y, color);
		set_pixel(_x, y+h, color);
    }
    for (int _y = y; _y < y+h; _y++){
		
		set_pixel(x, _y, color);
		set_pixel(x+w, _y, color);
    }
}