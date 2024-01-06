#pragma once

typedef struct env {
    int OS_VERSION_ENV;                   ///< Версия ENV
    int OS_VERSION_MAJOR;                 ///< Версия ядра
    int OS_VERSION_MINOR;                 ///< Пре-Релиз
    int OS_VERSION_PATCH;                  ///< Патч
    size_t DisplayFrameBuffer_Address;                ///< Ссылка на виртуальный экран
    size_t Link_Time;                   ///< Ссылка на время
    size_t Display_W;                        ///< Длина экрана
    size_t Display_H;                        ///< Высота экрана
    size_t Display_B;                        ///< Бит / пиксель
    size_t Display_P;                        ///< Глубина экрана
    size_t Display_S;                        ///< Размер буфера
    size_t Ticks;                            ///< Количество текущих тиков
    size_t RAM_Install;                 ///< Установлено ОЗУ
    size_t RAM_Used;                    ///< Использовано ОЗУ
    size_t RAM_Free;                    ///< Свободно ОЗУ
} env_t;

extern env_t system_environment;

void configure_env();