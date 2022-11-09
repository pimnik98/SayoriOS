/**
 * @file sys/sync.c
 * @author Пиминов Никита (nikita.piminoff@yandex.ru)
 * @brief Примитивы синхронизации
 * @version 0.3.0
 * @date 2022-10-01
 * @copyright Copyright SayoriOS Team (c) 2022
 */
#include	"sys/sync.h"

/**
 * @brief Получить мьютекс
 * 
 * @param mutex_t* mutex - Мьютекс
 * @param bool wait - Время ожидания
 *
 * @return bool
 */
bool mutex_get(mutex_t* mutex, bool wait){
	bool old_value = true;

	do
	{
		asm volatile ("xchg (,%1,), %0":"=a"(old_value):"b"(mutex), "a"(old_value));

	} while (old_value && wait);

	return !old_value;
}

/**
 * @brief Получить ближайщий свободный блок
 * 
 * @param mutex_t* mutex - Мьютекс
 *
 * @return physmemory_pages_block_t* - Блок
 */
void mutex_release(mutex_t* mutex){
	*mutex = false;
}
