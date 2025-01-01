/**
 * @file sys/sync.c
 * @author Пиминов Никита (nikita.piminoff@yandex.ru)
 * @brief Примитивы синхронизации
 * @version 0.3.5
 * @date 2022-10-01
 * @copyright Copyright SayoriOS Team (c) 2022-2024
 */
#include	"sys/sync.h"

/**
 * @brief Получить мьютекс
 * 
 * @param mutex - Мьютекс
 * @param wait - Время ожидания
 *
 * @return bool
 */
bool mutex_get(mutex_t* mutex, bool wait){
	bool old_value = true;

	do {
		__asm__ volatile ("xchg (,%1,), %0":"=a"(old_value):"b"(mutex), "a"(old_value));
	} while (old_value && wait);

	return !old_value;
}

/**
 * @brief Получить ближайщий свободный блок
 * 
 * @param mutex - Мьютекс
 *
 * @return physmemory_pages_block_t* - Блок
 */
void mutex_release(mutex_t* mutex){
	*mutex = false;
}