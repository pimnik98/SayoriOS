/**
 * @file lib/php/explode.c
 * @author Пиминов Никита (nikita.piminoff@yandex.ru)
 * @brief Функция замены строк
 * @version 0.3.5
 * @date 2023-07-30
 * @copyright Copyright SayoriOS Team (c) 2022-2024
 */
#include <mem/vmm.h>
#include "lib/string.h"
#include "lib/php/explode.h"

/**
 * @brief Функция отладки
 *
 * @param a_str - Строка для деления
 * @param del - Делитель (только 1 символ)
 *
 * @return uint32_t - Количество строк
 */
uint32_t str_cdsp2(const char a_str[], char del){
    int x = 0;
    for(size_t i = 0, len = strlen(a_str); i < len; i++){
        if (a_str[i] == del) {
            x++;
        }
    }
    return x;
}

// TODO: Remake explode to work with libvector and libstring!
char** explode(const char str[], char delimiter) {
	uint32_t ccc = str_cdsp2(str, delimiter);
	char** result = kmalloc(strlen(str)*2);
	int y = 0;
	int a = 0;

	for (int b = 0; b <= ccc; b++) {
		result[b] = kmalloc(strlen(str) * sizeof(char));
	}	

	for (int i = 0; i < strlen(str); i++){
		if (str[i] == delimiter){
			result[a][y] = 0;
			a++;
			y = 0;
			continue;
		}
 		result[a][y] = str[i];
		y++;
	}

	result[a][y] = 0;
    return result;
}