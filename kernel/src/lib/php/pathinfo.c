/**
 * @file lib/php/pathinfo.c
 * @author Пиминов Никита (nikita.piminoff@yandex.ru)
 * @brief Функция замены строк
 * @version 0.3.5
 * @date 2023-07-30
 * @copyright Copyright SayoriOS Team (c) 2022-2024
 */

#include <lib/php/pathinfo.h>
#include "mem/vmm.h"

char* pathinfo(const char* Path, int Mode){
	int i, length = strlen(Path), dot_index = -1;
	char* info = kcalloc(length + 1, 1);
	if (Mode == PATHINFO_DIRNAME) {
		for (i = length - 1; i >= 0; i--) {
			if (Path[i] == '/' || Path[i] == '\\') {
				break;
			}
		}
		if (i >= 0) {
			strncpy(info, Path, i + 1);
			info[i + 1] = '\0';
		} else {
			info[0] = '\0';
		}
	} else if (Mode == PATHINFO_BASENAME){
		for (i = length - 1; i >= 0; i--) {
			if (Path[i] == '/' || Path[i] == '\\') {
				break;
			}
		}
	
		if (i >= 0) {
			strncpy(info, Path + i + 1, length - i);
			info[length - i] = '\0';
		} else {
			strcpy(info, Path);
		}
	} else if (Mode == PATHINFO_EXTENSION){
		for (i = length - 1; i >= 0; i--) {
			if (Path[i] == '.') {
				break;
			}
		}
	
		if (i >= 0 && i < length - 1) {
			strncpy(info, Path + i + 1, length - i);
			info[length - i] = '\0';
		} else {
			info[0] = '\0';
		}
	} else {
		for (i = length - 1; i >= 0; i--) {
			if (Path[i] == '.') {
				dot_index = i;
				break;
			}
		}
	
		if (dot_index >= 0) {
			strncpy(info, Path, dot_index);
			info[dot_index] = '\0';
		} else {
			// Если точка не найдена, возвращаем полное имя файла
			strcpy(info, Path);
		}
	}
	return info;
}