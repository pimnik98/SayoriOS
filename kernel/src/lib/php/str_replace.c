/**
 * @file lib/php/str_replace.c
 * @author Пиминов Никита (nikita.piminoff@yandex.ru)
 * @brief Функция замены строк
 * @version 0.3.4
 * @date 2023-07-30
 * @copyright Copyright SayoriOS Team (c) 2022-2023
 */
#include "common.h"
#include "lib/string.h"

/**
 * @brief Заменяет все вхождения символа поиска на символ замены
 *
 * @param search - Искомое значение, также известное как needle (иголка).
 * @param replace - Значение замены, будет использовано для замены искомых значений search.
 * @param subject - Строка, в котором производится поиск и замена, также известный как haystack (стог сена).
 *
 * @return uint32_t - Количество произведенных замен
 */
uint32_t char_replace(char search, char replace, char* subject){
	size_t x = 0;
    for(size_t i = 0; i < strlen(subject); i++){
        if (subject[i] == search) {
			subject[i] = replace;
            x++;
        }
    }
    return x;
}

/**
 * @brief Заменяет все вхождения строки поиска на строку замены
 * 
 * @warning Не готово
 *
 * @param search - Искомое значение, также известное как needle (иголка).
 * @param replace - Значение замены, будет использовано для замены искомых значений search.
 * @param subject - Строка, в котором производится поиск и замена, также известный как haystack (стог сена).
 *
 * @return uint32_t - Количество произведенных замен
 */
uint32_t str_replace(char* search, char* replace, char* subject){
    return 0;
}