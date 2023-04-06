/**
 * @file drv/env.c
 * @author Пиминов Никита (nikita.piminoff@yandex.ru)
 * @brief Пользовательское окружение ОС
 * @version 0.3.2
 * @date 2022-11-07
 * @copyright Copyright SayoriOS Team (c) 2022-2023
*/

#include <kernel.h>
#include <io/ports.h>
#include "user/env.h"

struct env e = {0};

/**
 * @brief Печатает в логи, состояние об окружении ядра
 */
struct env* printEnv(){
	// qemu_log("Address of env struct: %x", &e);
    return &e;
}

void confidEnv(){
    qemu_log("[ENV] Config...");
    e.OS_VERSION_MAJOR = VERSION_MAJOR;                 ///< Версия ядра
    e.OS_VERSION_MINOR = VERSION_MINOR;                 ///< Пре-Релиз
    e.OS_VERSION_PATCH = VERSION_PATCH;                  ///< Патч
    e.Link_Display = getFrameBufferAddr();                     ///< Ссылка на виртуальный экран
    e.Link_Time = 2;                        ///< Ссылка на время
    e.Display_W = getWidthScreen();                        ///< Длина экрана
    e.Display_H = getHeightScreen();                        ///< Высота экрана
    e.Display_B = getDisplayBpp();                        ///< Смещение экрана
    e.Display_P = getDisplayPitch();                        ///< Глубина экрана
    e.Display_S = getDisplaySize();                        ///< Размер буфера
    e.Ticks = getTicks();                            ///< Количество текущих тиков
    e.RAM_Install = getInstalledRam();                      ///< Установлено ОЗУ
    e.RAM_Used = memory_get_used_kernel();                         ///< Использовано ОЗУ
    e.RAM_Free = 0;                         ///< Свободно ОЗУ
    e.OS_VERSION_ENV = 1;
    // char Username[128];                   ///< Имя пользователя
    // char Hostname[128];                   ///< Имя устройства
    // char CPU_Name[128];                   ///< Имя процессора
    // char OS_Name[128];                    ///< Имя ОС
    // char OS_CodeName[128];                ///< Кодовое название ОС
    // char OS_Build[128];                   ///< Дата компиляции
    // char OS_Arch[128];                    ///< Архитектура ядра
    qemu_log("[ENV] Version %d environment completed successfully.", e.OS_VERSION_ENV);
}
