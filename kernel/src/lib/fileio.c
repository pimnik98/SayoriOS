/**
 * @file lib/fileio.c
 * @authors Пиминов Никита (nikita.piminoff@yandex.ru), NDRAEY >_ (pikachu_andrey@vk.com)
 * @brief Функции для работы с файлами и папками
 * @version 0.3.5
 * @date 2022-11-01
 * @copyright Copyright SayoriOS Team (c) 2022-2024
 */

#include "io/ports.h"
#include "mem/vmm.h"
#include "../../include/lib/fileio.h"
#include "../../include/fs/fsm.h"
#include "../../include/fs/nvfs.h"


/**
 * @brief [FileIO] Проверяет существует ли сущность и является ли она файлом
 *
 * @param Path - Путь
 *
 * @return bool - true - если успешно, в противном случае false
 */
bool is_file(const char* Path){
    FSM_FILE file = nvfs_info(Path);
    if (file.Ready != 1) return false;
    if (file.Type != 0) return false;
    return true;
}
