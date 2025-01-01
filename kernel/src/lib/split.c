/**
 * @file lib/split.c
 * @author Пиминов Никита (nikita.piminoff@yandex.ru)
 * @brief Функция для деления строк
 * @version 0.3.5
 * @date 2022-11-01
 * @copyright Copyright SayoriOS Team (c) 2022-2024
 */
#include <mem/vmm.h>

#include "common.h"
#include "lib/string.h"

/**
 * @brief Функция отладки
 *
 * @param a_str - Строка для деления
 * @param del - Делитель (только 1 символ)
 *
 * @return uint32_t - Количество строк
 */
uint32_t str_cdsp(const char *a_str, const char* del){
    int x = 0;
    for(size_t i = 0, len = strlen(a_str); i < len; i++){
        if (a_str[i] == del[0]) {
            x++;
        }
    }
    return x;
}

/**
 * @brief Функция для деления строк
 *
 * @param a_str - Строка для деления
 * @param out - Здесь будет результат работы
 * @param del - Делитель (только 1 символ)
 *
 */
void str_split(const char a_str[], char *out[], char* del){
	size_t x = str_cdsp(a_str, del);
    // char* copy = kcalloc(strlen(a_str) + 1, 1);
    // memcpy(copy, a_str, strlen(a_str));

    int i = 0;
    char *p;
    p = strtok(a_str, del);
    if (p == nullptr){
        out[i] = a_str;
    }
    out[i] = p;
    i++;

    for(uint32_t a = 0; a < x; a++){
        p = strtok(out[i], del);
        if (p != nullptr){
            out[i] = p;
            i++;
        }
    }
}
