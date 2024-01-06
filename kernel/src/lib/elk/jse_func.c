/// JavaScript Engine - Спомогательные функции
#include "portability.h"
#include <io/ports.h>


#include "io/tty.h"
#include "lib/libstring/string.h"
#include "../libvector/include/vector.h"
#include "lib/stdio.h"


#include "elk_config.h"
#include "elk.h"

char* jse_strstr(const char* haystack, const char* needle) {
    if (*needle == '\0') {
        return (char*) haystack;
    }

    while (*haystack) {
        const char* h = haystack;
        const char* n = needle;

        while (*h && *n && (*h == *n)) {
            h++;
            n++;
        }

        if (*n == '\0') {
            return (char*) haystack;
        }

        haystack++;
    }

    return NULL;
}

void jse_ncpy(char *destination, const char *source, int length) {
    for (int i = 0; i < length; i++) {
        if (source[i] != '\0') {
            destination[i] = source[i]; // Копируем символ из source в destination
        } else {
            destination[i] = '\0'; // Дополняем destination нулевыми символами, если source закончилась
        }
    }
}


void jse_trim(char *str) {
    int start = 0, end = strlen(str) - 1;

    // Удаляем пробелы в начале строки
    while (str[start] == ' ' || str[start] == '\t' || str[start] == '\n') {
        start++;
    }

    // Удаляем пробелы в конце строки
    while (str[end] == ' ' || str[end] == '\t' || str[end] == '\n') {
        end--;
    }

    // Сдвигаем символы, чтобы удалить пробелы в начале
    for (int i = 0; i <= end - start; i++) {
        str[i] = str[i + start];
    }

    // Устанавливаем конец строки
    str[end - start + 1] = '\0';
}

// Функция для совмещения двух буферов с текстом
char* jse_mergeBuffers(char *buffer1, char *buffer2, int bufferSize1, int bufferSize2) {
    //qemu_log("\n\nbuf1: [%d] '%s'\n\n", bufferSize1, buffer1);
    //qemu_log("\n\nbuf2: [%d] '%s'\n\n", bufferSize2, buffer2);
    int mergedSize = bufferSize1 + bufferSize2 ; // Вычисляем размер объединенного буфера
    char *mergeResult = (char *)calloc(1, (mergedSize + 1) * sizeof(char)); // Выделяем память под новый буфер

    // Копируем содержимое первого буфера в объединенный буфер
    jse_ncpy(mergeResult, buffer1, bufferSize1);

    // Копируем содержимое второго буфера в объединенный буфер
    jse_ncpy(mergeResult + bufferSize1, buffer2, bufferSize2);

    // Устанавливаем завершающий нулевой символ
    mergeResult[mergedSize] = '\0';

    // Освобождаем память, занятую первым буфером
    free(buffer1);
    // Освобождаем память, занятую вторым буфером
    // Я сам освобождаю его в другом месте, поэтому тут оно не надо
    //free(buffer2);

    return mergeResult;
}

/**
 * @brief Перевод строки в нижний регистр
 *
 * @param as - Указатель на строку.
 */
void jse_func_tolower(char* as){
    while(*as != 0)
    {
        if(*as >= 'A' && *as <= 'Z')
            *as += ('a' - 'A');
        as++;
    }
}

/**
 * @brief Перевод строки в верхний регистр
 *
 * @param as - Указатель на строку.
 */
void jse_func_toupper(char* as){
    while(*as != 0)
    {
        if(*as >= 'a' && *as <= 'z')
            *as -= ('a' - 'A');
        as++;
    }
}

char jse_func_char_tolower(char ch) {
    if (ch >= 'A' && ch <= 'Z') {
        return ch + 32;  // Разница между большой и маленькой буквой в ASCII
    } else {
        return ch;
    }
}

int jse_func_atoi(const char* str) {
    int result = 0;
    int sign = 1;
    int base = 10;  // По умолчанию десятичная система

    // Пропускаем начальные пробелы
    while (*str == ' ') {
        str++;
    }

    // Обрабатываем знак +/-
    if (*str == '-') {
        sign = -1;
        str++;
    } else if (*str == '+') {
        str++;
    }

    // Обработка префиксов 0x (шестнадцатеричное число) и 0 (восьмеричное число)
    if (*str == '0') {
        str++;
        if (*str == 'x' || *str == 'X') {
            base = 16;  // Шестнадцатеричная система
            str++;
        } else {
            base = 8;  // Восьмеричная система
        }
    }

    // Обработка числовых символов
    while (*str) {
        int digit = *str - '0';
        if (digit >= 0 && digit < base) {
            result = result * base + digit;
        } else if (base == 16) {
            int hex = *str - 'A' + 10;
            if (hex >= 10 && hex < 16) {
                result = result * base + hex;
            } else {
                break;
            }
        } else {
            break;
        }
        str++;
    }

    return sign * result;
}


/// Unused
int jse_p_int(const char* str, char** endptr){
    int result = 0;
    bool negative = false;
    int exponent = 0;
    int sign = 1;

    // Пропускаем начальные пробелы
    while (*str == ' ') {
        str++;
    }

    // Определяем знак числа
    if (*str == '-' || *str == '+') {
        sign = (*str++ == '-') ? -1 : 1;
    }

    // Парсим целую часть числа
    while (*str >= '0' && *str <= '9') {
        result = result * 10 + (*str++ - '0');
    }

    // Парсим дробную часть числа
    if (*str == '.') {
        //double fraction = 1.0;
        str++;
        while (*str >= '0' && *str <= '9') {
            //result = result + (double)(*str - '0') / (fraction *= 10.0);
            str++;
        }
    }

    result *= sign;

    if (endptr != NULL) {
        *endptr = (char*)str;
    }

    return result;
}

int jse_getInt(struct js *js, jsval_t arg){
    int type = js_type(arg);
    if (type == JS_NUM) return js_getnum(arg);
    if (type == JS_STR) return jse_func_atoi(js_str(js,arg));
    if (type == JS_UNDEF) return 0;
    if (type == JS_NULL) return 0;
    if (type == JS_FALSE) return 0;
    if (type == JS_TRUE) return 1;
    return js_mkundef();
}

/**
 * @brief Функция strdup дублирует строку, на которую указывает аргумент str. Память под дубликат строки выделяется с помощью функции malloc, и по окончанию работы с дубликатом должна быть очищена с помощью функции free
 *
 * @param str
 *
 * @return char* строка
 */
char* jse_strdup(const char* str) {
    size_t length = strlen(str) + 1; // Длина строки + 1 для символа '\0'
    char* newStr = calloc(1,length); // Выделение памяти для новой строки

    if (newStr != NULL) {
        memcpy(newStr, str, length); // Копирование содержимого исходной строки
        newStr[length] = 0;
    }

    return newStr;
}