/**
 * @file sys/variable.c
 * @authors Пиминов Никита (github.com/pimnik98 | VK: @piminov_remont)
 * @brief Система переменных
 * @version 0.3.5
 * @date 2023-06-01
 *
 * @copyright Copyright SayoriOS Team (c) 2022-2024
 *
 */
#include <mem/vmm.h>
#include "lib/php/str_contains.h"
#include <io/ports.h>
#include <sys/variable.h>  

VARIABLE G_VARIABLE[128] = {0};
size_t C_VARIABLE = 0;

size_t variable_freeID(char* Key){
	bool Key404 = false;
	for (int i = 0; i < 512;i++){
		/// Сначала ищем по ключу, если не найдена, то ищем пустую ячейку
		if (strcmpn(G_VARIABLE[i].Key,Key)) return i;
		if (G_VARIABLE[i].Ready == 0 && Key404) return i;
		if (i == 511 && !Key404){
			Key404 = true;
			i = 0;
		}
	}
	return -1;
}

int variable_write(char* Key, char* Value){
	qemu_log("[Variable] [Write] %s=%s",Key,Value);
	size_t inx = variable_freeID(Key);
	if (inx == -1){
		qemu_log("[Variable] [Error] Return index: %d",inx);
		return 0;
	}
	if (G_VARIABLE[inx].Ready == 1 && strlen(Value) == 0){
		C_VARIABLE--;
	} else if (G_VARIABLE[inx].Ready == 0 && strlen(Value) != 0){
		C_VARIABLE++;
	}
	G_VARIABLE[inx].Ready = (strlen(Value) == 0?0:1);
	memcpy(G_VARIABLE[inx].Key,Key,strlen(Key));
	memcpy(G_VARIABLE[inx].Value,Value,strlen(Value));
	return 1;
}

char* variable_read(char* Key){
	qemu_log("[Variable] [Read] %s",Key);
	for (int i = 0; i < 512;i++){
		if (G_VARIABLE[i].Ready && strcmpn(G_VARIABLE[i].Key,Key)){
			qemu_log("[Variable] [Read] %s=%s",G_VARIABLE[i].Key,G_VARIABLE[i].Value);
			return G_VARIABLE[i].Value;
		}
	}
	qemu_log("[Variable] [Read] %s=NULL",Key);
	return NULL;
}

VARIABLE* variable_list(char* Search){
	qemu_log("[Variable] [List] [ALL=%d] Search: %s",C_VARIABLE,Search);
	VARIABLE* list = kmalloc(sizeof(VARIABLE)*(C_VARIABLE+1));
	size_t inx = 0;
	for (int i = 0; i < 512;i++){
		if (G_VARIABLE[i].Ready == 0) continue;
		qemu_log("[%d] Ready: %d | Search:%d | Key:%s",inx, G_VARIABLE[i].Ready, str_contains(G_VARIABLE[i].Key,Search), G_VARIABLE[i].Key);
		if (str_contains(G_VARIABLE[i].Key,Search) == 0) continue;
		list[inx].Ready = 1;
		memcpy(list[inx].Key,G_VARIABLE[i].Key,strlen(G_VARIABLE[i].Key));
		memcpy(list[inx].Value,G_VARIABLE[i].Value,strlen(G_VARIABLE[i].Value));
		inx++;
	}
	list[inx].Ready = 0;
	return list;
}
