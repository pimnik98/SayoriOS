/**
 * @file lib/split.c
 * @author Пиминов Никита (nikita.piminoff@yandex.ru)
 * @brief Функция для деления строк
 * @version 0.3.0
 * @date 2022-11-01
 * @copyright Copyright SayoriOS Team (c) 2022
 */
#include <kernel.h>

/**
 * @brief Функция откладки
 *
 * @param char* a_str - Строка для деления
 * @param char* del - Делитель (только 1 символ)
 *
 * @return uint32_t - Количество строк
 */
uint32_t str_cdsp(char* a_str, char* del){
    int x = 0;
    for(int i=0; i < strlen(a_str); i++){
        if (a_str[i] == del[0]) {
            x++;
        }
    }
    return x;
}

/**
 * @brief Функция для деления строк
 *
 * @param char* a_str[] - Строка для деления
 * @param char *out[] - Здесь будет результат работы
 * @param char* del[] - Делитель (только 1 символ)
 *
 * @return char* - Здесь тоже будет результут работы
 */
char* str_split(char a_str[], char *out[], char* dec){
    int x = str_cdsp(a_str,dec);

    int i = 0;
    char *p;
    p = strtok(a_str, dec);
    if (p == NULL){
        out[i] = a_str;
        return out;
    }
    out[i] = p;
    i++;

    for(uint32_t a = 0;a < x; a++){
        p = strtok(out[i], dec);
        if (p != NULL){
            out[i] = p;
            i++;
        }
    }
    return out;
}
