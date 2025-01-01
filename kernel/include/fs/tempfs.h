#ifndef SAYORIOS_TEMPFS_H
#define SAYORIOS_TEMPFS_H

#include "../../include/portability.h"
#include <stdarg.h>

#define tfs_log(M,...); printf(M,##__VA_ARGS__);

#define TEMPFS_CHMOD_EXEC               0x01  /// Права выполнения
#define TEMPFS_CHMOD_WRITE              0x02  /// Права записи
#define TEMPFS_CHMOD_READ               0x04  /// Права чтения
#define TEMPFS_CHMOD_SYS                0x08  /// Права системы


#define TEMPFS_DIR_INFO_ROOT            0x01  /// Родительская папка найдена
#define TEMPFS_DIR_INFO_EXITS           0x02  /// Папка найдена

#define TEMPFS_ENTITY_STATUS_ERROR      0x00  /// Сущность НЕ готова к работе
#define TEMPFS_ENTITY_STATUS_READY      0x01  /// Сущность готова к работе (entity)
#define TEMPFS_ENTITY_STATUS_PKG_READY  0x02  /// Сущность готова к работе (package)

#define TEMPFS_ENTITY_TYPE_UNKNOWN      0x00  /// Неизвестно
#define TEMPFS_ENTITY_TYPE_FILE         0x01  /// Файл
#define TEMPFS_ENTITY_TYPE_FOLDER       0x02  /// Папка


typedef struct {
    uint8_t Status;			///< Статус
    char Name[64];		    ///< Имя файла | папки
    char Path[412];		    ///< Путь файла | папки
    uint32_t Size;			///< Размер файла в байтах (только для файлов)
    uint32_t Point;			///< Точка входа в файл | папки
    uint32_t Date;			///< Дата изменения
    char Owner[16];		    ///< Владелец файла | папки
    uint8_t Type;			///< Тип файл или папки
    uint8_t CHMOD;			///< Права доступа
} TEMPFS_ENTITY;  // Получится 512 байт

typedef struct {
    uint16_t Sign1;			///< Сигнатура 1
    uint16_t Sign2;			///< Сигнатура 2
    char Label[32];		    ///< Метка диска
    uint32_t CountFiles;	///< Количество файлов
    uint32_t EndDisk;		///< Точка конца диска
    uint32_t CountBlocks;	///< Количество используемых блоков
    char Rev[16];			///< Не используется
}  TEMPFS_BOOT;

typedef struct {
    uint8_t Status;					///< Статус пакета
    uint16_t Length;				///< Длина пакета
    uint32_t Next;					///< Следующий пакет данных
    char Data[500];					///< Пакет данных
    uint8_t Rev;					///< Зарезервировано, всегда ноль
} TEMPFS_PACKAGE;  // Получится 512 байт

typedef struct {
    uint8_t Status;			    ///< Статус
    TEMPFS_BOOT* Boot;			///< Ссылка на Boot
    size_t CountFiles;	        ///< Количество файлов
    TEMPFS_ENTITY* Files;		///< Ссылка на Файловые поинты
    size_t BlocksAll;	        ///< Максимальное количество блоков информации
    size_t FreeAll;             ///< Свободное количество блоков
    size_t EndPoint;            ///< Точка конца диска
}  TEMPFS_Cache;

FSM_FILE fs_tempfs_info(const char Disk, const char* Path);
FSM_DIR* fs_tempfs_dir(const char Disk, const char* Path);
int fs_tempfs_create(const char Disk,const char* Path,int Mode);
int fs_tempfs_delete(const char Disk,const char* Path,int Mode);
void fs_tempfs_label(const char Disk, char* Label);
int fs_tempfs_detect(const char Disk);
void fs_tempfs_format(const char Disk);
size_t fs_tempfs_write(const char Disk, const char* Path, size_t Offset, size_t Size, void* Buffer);
size_t fs_tempfs_read(const char Disk, const char* Path, size_t Offset, size_t Size, void* Buffer);

#define TMF_GETDISKSIZE(Ch) dpm_disk_size(Ch)

#endif //SAYORIOS_TEMPFS_H
