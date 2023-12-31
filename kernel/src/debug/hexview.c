#include "common.h"

/**
 * @brief Печатает дамп памяти
 * @param buffer Место в памяти откуда нужно печать дамп
 * @param length Длина куска памяти
 * @param width Сколько байт нужно показать в одной строке
 * @param relative Показывает относительный адрес если true иначе показывает абсолютный
 * @param printer_func Функция для печати (должна поддерживать форматирование)
 */
void hexview_advanced(void *buffer, size_t length, size_t width, bool relative, void (*printer_func)(const char *, ...)) {
    char* cbuf = (char*)buffer;

	for(size_t i = 0; i < length; i += width) {
        if(relative)
            printer_func("%08v: ", i);
        else
            printer_func("%08v: ", cbuf + i);

        for(int j = 0; j < (length - i < width ? length - i : width); j++) {
            printer_func("%02v ", ((char)*(cbuf + i + j)) & 0xFF);
        }

        printer_func("\n");
    }
}