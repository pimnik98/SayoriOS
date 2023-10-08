#pragma once

#include <common.h>

enum SMFS_PACKAGE_STATUS {
	SMFS_PACKAGE_UNKNOWN	= 0,	///< Статус неизвестен
	SMFS_PACKAGE_READY		= 1,	///< Пакет в порядке
	SMFS_PACKAGE_FREE		= 2,	///< Пакет готов для записи
};

enum SMFS_ELEM_TYPE {
	SMFS_TYPE_UNKNOWN		= 0,	///< Да, хуй его знает
	SMFS_TYPE_FILE			= 1,	///< ФАЙЛ
	SMFS_TYPE_DIR			= 2,	///< Папка
	SMFS_TYPE_DELETE		= 3,	///< Ячейка свободна
};

typedef struct {
    uint16_t magic1;				///< Подпись загрузочного раздела
    uint16_t magic2;				///< Подпись загрузочного раздела
    char     oem_name[8];			///< ОЕМ-идентификатор.
    uint32_t MaximumElems;			///< Максимальное колво элементов
    uint32_t MaxPackage;			///< Количество пакетов
    char     volume_label[11];		///< Метка
    char     fsid[8];				///< Строка идентификатора системы.
} __attribute__((__packed__)) SMFS_BOOT_SECTOR; 

typedef struct {
	uint32_t Index;					///< Индекс элемента
	uint8_t Attr;					///< Атрибуты элемента
	uint32_t Size;					///< Размер элемента
	uint16_t TimeCreateHIS;			///< Время создания файла. Умножьте секунды на 2. (Часы 5 / Минуты 6 / Секунды 5)
    uint16_t TimeCreateDate;		///< Дата создания файла (Год 7 / Месяц 4 / День 5)
    uint16_t TimeAccess;			///< Дата последнего доступа. Тот же формат, что и дата создания. всегда равно нулю.
    uint32_t Point;					///< Точка входа
    uint32_t Dir;					///< Папка
	char Name[32];					///< 28 - для имени + 3 для расширения
} __attribute__((__packed__)) SMFS_Elements; 

typedef struct {
	uint8_t Status;					///< Статус пакета
	uint8_t Length;					///< Длина пакета
	char Data[9];					///< Пакет данных
	uint32_t Next;					///< Следующий пакет данных
} __attribute__((__packed__)) SMFS_PACKAGE;  // Получится 15 байт

typedef struct {
	uint8_t Status;					///< Статус пакета
	uint8_t Length;					///< Длина пакета
	char Data[33];					///< Пакет данных
	uint32_t Next;					///< Следующий пакет данных
} __attribute__((__packed__)) SMFS_PACKAGE_BIG;  // Получится 39 байт