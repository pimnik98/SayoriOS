#pragma once

typedef struct env
{
  char Username[128];                   ///< Имя пользователя
  char Hostname[128];                   ///< Имя устройства
  char CPU_Name[128];                   ///< Имя процессора
  char OS_Name[128];                    ///< Имя ОС
  char OS_CodeName[128];                ///< Кодовое название ОС
  char OS_Build[128];                   ///< Дата компиляции
  char OS_Arch[128];                    ///< Архитектура ядра
  int OS_VERSION_ENV;                   ///< Версия ENV
  int OS_VERSION_MAJOR;                 ///< Версия ядра
  int OS_VERSION_MINOR;                 ///< Пре-Релиз
  int OS_VERSION_PATCH;                  ///< Патч
  size_t Link_Display;                ///< Ссылка на виртуальный экран
  size_t Link_Time;                   ///< Ссылка на время
  int Display_W;                        ///< Длина экрана
  int Display_H;                        ///< Высота экрана
  int Display_B;                        ///< Смещение экрана
  int Display_P;                        ///< Глубина экрана
  int Display_S;                        ///< Размер буфера
  int Ticks;                            ///< Количество текущих тиков
  size_t RAM_Install;                 ///< Установлено ОЗУ
  size_t RAM_Used;                    ///< Использовано ОЗУ
  size_t RAM_Free;                    ///< Свободно ОЗУ
} env_t;

struct env* printEnv();
