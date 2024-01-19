/**
 * @file drv/env.c
 * @author Пиминов Никита (nikita.piminoff@yandex.ru)
 * @brief Пользовательское окружение ОС
 * @version 0.3.5
 * @date 2022-11-07
 * @copyright Copyright SayoriOS Team (c) 2022-2024
*/

#include <version.h>
#include <io/ports.h>
#include "user/env.h"
#include "io/screen.h"
#include "mem/vmm.h"
#include "mem/pmm.h"
#include "sys/timer.h"

struct env system_environment = {0};

void configure_env(){
    system_environment.OS_VERSION_MAJOR = VERSION_MAJOR;                 ///< Версия ядра
    system_environment.OS_VERSION_MINOR = VERSION_MINOR;                 ///< Пре-Релиз
    system_environment.OS_VERSION_PATCH = VERSION_PATCH;                  ///< Патч
    system_environment.DisplayFrameBuffer_Address = getFrameBufferAddr();                     ///< Ссылка на виртуальный экран
    system_environment.Display_W = getScreenWidth();                        ///< Длина экрана
    system_environment.Display_H = getScreenHeight();                        ///< Высота экрана
    system_environment.Display_B = getDisplayBpp();                        ///< Смещение экрана
    system_environment.Display_P = getDisplayPitch();                        ///< Глубина экрана
    system_environment.Display_S = getDisplaySize();                        ///< Размер буфера
    system_environment.Ticks = getTicks();                            ///< Количество текущих тиков
    system_environment.RAM_Install = phys_memory_size;                      ///< Установлено ОЗУ
    system_environment.RAM_Used = used_phys_memory_size;                         ///< Использовано ОЗУ
    system_environment.RAM_Free = phys_memory_size - used_phys_memory_size;                         ///< Свободно ОЗУ
}
