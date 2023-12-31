/**
 * @file lib/php/pathinfo.c
 * @author Пиминов Никита (nikita.piminoff@yandex.ru)
 * @brief Функция замены строк
 * @version 0.3.4
 * @date 2023-07-30
 * @copyright Copyright SayoriOS Team (c) 2022-2023
 */
#pragma once

#include "lib/string.h"

static inline bool str_contains(const char* haystack, const char* needle) {
	const char* result = strstr(haystack, needle);

	return result != NULL;
}