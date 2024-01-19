/**
 * @file lib/pixel.c
 * @author Пиминов Никита (nikita.piminoff@yandex.ru)
 * @brief Драйвера для работы с пикселями (Пакет Пиксель)
 * @version 0.3.5
 * @date 2022-10-20
 * @copyright Copyright SayoriOS Team (c) 2022-2024
 */

#include <io/ports.h>
#include <io/status_loggers.h>
#include <lib/stdio.h>
#include <lib/tui.h>
#include <lib/pixel.h>

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
	for (__typeof__(x) _y = y, endy = y+h; _y < endy; _y++){
		for (__typeof__(x) _x = x, endx = x+w; _x < endx; _x++){
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

/**
 * @brief Рисует горизонтальную линию
 *
 * @param x1 - Начальная координата X
 * @param x2 - Конечная координата x
 * @param y - Высота
 * @param color - Цвет
*/
void drawHorizontalLine(int x1, int x2, int y, uint32_t color) {
    for (int x = x1; x <= x2; x++) {
        set_pixel(x, y, color);
    }
}

/**
 * @brief Рисует вертикальную линию
 *
 * @param y1 - Начальная координата Y
 * @param y2 - Конечная координата Y
 * @param x - Позиция по X
 * @param color - Цвет
*/
void drawVerticalLine(int y1, int y2, int x, uint32_t color) {
    for (int y = y1; y <= y2; y++) {
        set_pixel(x, y, color);
    }
}

/**
 * @brief Рисует окружные пиксели круга линию
 *
 * @warning Спомогательная функция!
 *
 * @param cx - Позиция по X
 * @param cy - Позиция по Y
 * @param x - Смещение по X
 * @param y - Смещение по Y
 * @param color - Цвет
*/
void drawCirclePoints(int cx, int cy, int x, int y, uint32_t color) {
    set_pixel(cx + x, cy + y, color);
    set_pixel(cx - x, cy + y, color);
    set_pixel(cx + x, cy - y, color);
    set_pixel(cx - x, cy - y, color);
    set_pixel(cx + y, cy + x, color);
    set_pixel(cx - y, cy + x, color);
    set_pixel(cx + y, cy - x, color);
    set_pixel(cx - y, cy - x, color);
}

/**
 * @brief Рисует круг
 *
 * @param cx - Позиция по X
 * @param cy - Позиция по Y
 * @param radius - Радиус круга
 * @param color - Цвет
*/
void drawCircle(int cx, int cy, int radius, uint32_t color) {
    int x = 0;
    int y = radius;
    int p = 3 - 2 * radius;
    drawCirclePoints(cx, cy, x, y, color);

    while (x <= y) {
        if (p < 0) {
            p = p + 4 * x + 6;
        } else {
            p = p + 4 * (x - y) + 10;
            y--;
        }
        x++;
        drawCirclePoints(cx, cy, x, y, color);
    }
}


/**
 * @brief Закрашивает круг
 *
 * @param x0 - Позиция по X
 * @param y0 - Позиция по Y
 * @param radius - Радиус круга
 * @param color - Цвет
*/
void drawFilledCircle(int x0, int y0, int radius, uint32_t color) {
    int x = radius;
    int y = 0;
    int err = 0;

    while (x >= y) {
        drawHorizontalLine(x0 - x, x0 + x, y0 + y, color);
        drawHorizontalLine(x0 - x, x0 + x, y0 - y, color);
        drawHorizontalLine(x0 - y, x0 + y, y0 + x, color);
        drawHorizontalLine(x0 - y, x0 + y, y0 - x, color);

        if (err <= 0) {
            y += 1;
            err += 2*y + 1;
        }
        if (err > 0) {
            x -= 1;
            err -= 2*x + 1;
        }
    }
}

/**
 * @brief Закрашивает округленный куб
 *
 * @param x0 - Позиция по X
 * @param y0 - Позиция по Y
 * @param radius - Радиус круга
 * @param w - Максимальная длина в сторону, не должно быть равным 0
 * @param mode - Какая сторона?
 * @param color - Цвет
*/
void drawFilledRectBorder(int x0, int y0, int radius, int w, int mode, uint32_t color) {
    int x = radius;
    int y = 0;
    int err = 0;

    while (x >= y) {
        drawHorizontalLine(x0 - x - (mode == 3 || mode == 4?(w/2):0), x0 + x + (mode == 1 || mode == 2?(w/2):0) , y0 - y, color);
        drawHorizontalLine(x0 - y - (mode == 3 || mode == 4?(w/2):0), x0 + y + (mode == 1 || mode == 2?(w/2):0), y0 - x, color);
        drawHorizontalLine(x0 - x - (mode == 3 || mode == 4?(w/2):0), x0 + x + (mode == 1 || mode == 2?(w/2):0) , y0 + y, color);
        drawHorizontalLine(x0 - y - (mode == 3 || mode == 4?(w/2):0), x0 + y + (mode == 1 || mode == 2?(w/2):0), y0 + x, color);

        if (err <= 0) {
            y += 1;
            err += 2*y + 1;
        }
        if (err > 0) {
            x -= 1;
            err -= 2*x + 1;
        }
    }
}

/**
 * @brief Рисуем округленный куб закрашиваем его и обводим
 *
 * @param x - Позиция по X
 * @param y - Позиция по Y
 * @param size - Длина в стороны
 * @param radius - На сколько закругляем куб?
 * @param fill_color - Цвет закраски
 * @param border_color - Цвет обводки
*/
void drawRoundedSquare(int x, int y, int size, int radius, uint32_t fill_color, uint32_t border_color) {
    // Отрисовываем внутренний заполненный квадрат
    for (int i = x+1; i < x + size-1; i++) {
        drawVerticalLine(y + radius, y + size - radius - 1, i, fill_color);
    }


    // Отрисовываем угловые закругления
    if (radius > 0) {
        drawFilledRectBorder(x + radius, y + radius, radius, size, 1, (border_color != -1 ? border_color : fill_color));
        drawFilledRectBorder(x + size - radius - 1, y + radius, radius,size, 3, (border_color != -1 ? border_color : fill_color));
        drawFilledRectBorder(x + radius, y + size - radius - 1, radius,size, 2, (border_color != -1 ? border_color : fill_color));
        drawFilledRectBorder(x + size - radius - 1, y + size - radius - 1, radius,size, 4, (border_color != -1 ? border_color : fill_color));
    }

    // Отрисовываем обводку квадрата
    if (border_color != -1) {
        drawHorizontalLine(x + radius, x + size - 1 - radius, y, border_color);
        drawHorizontalLine(x + radius, x + size - 1 - radius, y + size - 1, border_color);
        drawVerticalLine(y + radius, y + size - 1 - radius, x, border_color);
        drawVerticalLine(y + radius, y + size - 1 - radius, x + size - 1, border_color);
    }
}


/**
 * @brief Рисуем округленный прямоугольник, закрашиваем его и обводим
 *
 * @param x - Позиция по X
 * @param y - Позиция по Y
 * @param width - Ширина
 * @param height - Высота
 * @param radius - Радиус закругления углов
 * @param fill_color - Цвет закраски
 * @param border_color - Цвет обводки
 */
void drawRoundedRectangle(int x, int y, int width, int height, int radius, uint32_t fill_color, uint32_t border_color) {
    // Отрисовываем внутренний заполненный прямоугольник
    for (int i = x + 1; i < x + width - 1; i++) {
        for (int j = y + radius; j < y + height - radius; j++) {
            set_pixel(i, j, fill_color);
        }
    }

    // Отрисовываем угловые закругления
    if (radius > 0) {
        drawFilledRectBorder(x + radius, y + radius, radius, width, 1, (border_color != -1 ? border_color : fill_color));
        drawFilledRectBorder(x + width - radius - 1, y + radius, radius,width, 3, (border_color != -1 ? border_color : fill_color));

        drawFilledRectBorder(x + radius, y + height - radius - 1, radius,width, 2, (border_color != -1 ? border_color : fill_color));
        drawFilledRectBorder(x + width - radius - 1, y + height - radius - 1, radius,width, 4, (border_color != -1 ? border_color : fill_color));
    }

}