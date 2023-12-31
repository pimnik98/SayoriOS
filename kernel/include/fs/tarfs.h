#pragma once

#include <common.h>
#define TARFS_ELEM_TYPE_FILE 48			///< Обычный файл
#define TARFS_ELEM_TYPE_HARD_LINK 49	///< Жесткая ссылка
#define TARFS_ELEM_TYPE_SYMB_LINK 50	///< Символическая ссылка
#define TARFS_ELEM_TYPE_CHR_DEV 51		///< Символьное устройство
#define TARFS_ELEM_TYPE_BLK_DEV 52		///< Блочное-устройство
#define TARFS_ELEM_TYPE_DIR 53			///< Папка
#define TARFS_ELEM_TYPE_PIPE 54			///< Канал (FIFO)

/**
 * @brief Структура файла
 */
typedef struct {
    char Name[100];			/// Имя файла
    char Mode[8];			/// Режим файла

    char UID[8];			/// Числовой идентификатор пользователя владельца
    char GID[8];			/// Числовой идентификатор пользователя группы

    char Size[12];			/// Размер файла в байтах (oсt2bin)
    char LastTime[12];		/// Время последнего изменения файла

    char CheckSum[8];		/// Контрольная сумма для записи заголовка
    char Type;				/// Тип элемента

    char Link[100];			/// Имя связанного файла
    char Signature[6];		/// Индикатор UStar
    char Version[2];		/// Версия Ustar

    char OwnerUser[32];		/// Имя владельца
    char OwnerGroup[32];	/// Имя группы

    char DM1[8];			/// Основной номер устройства
    char DM2[8];			/// Младший номер устройства

    char Prefix[155];		/// Префикс имени файла
} __attribute__((packed, aligned(512))) TarFS_Elem;

typedef struct {
	int Ready;				/// Существует ли файл?
    char Name[100];			/// Имя файла
    char Mode[8];			/// Режим файла
    size_t Size;			/// Размер файла в байтах (oсt2bin)
    char LastTime[12];		/// Время последнего изменения файла
    int Type;				/// Тип элемента
	uint32_t Addr;			/// Адрес размещения файла
	uint32_t Real;			/// Адрес размещения файла
} __attribute__((packed)) TarFS_File;

typedef struct {
	int Ready;				/// Инициализировано?
	size_t Count;			/// Кол-во файлов и элементов
	TarFS_File* Files;		/// Файлы
} __attribute__((packed)) TarFS_ROOT;

int oct2bin(char *str, int size);
TarFS_File tarfs_infoFile(TarFS_ROOT* r,const char* name);
char* tarfs_readFile(TarFS_ROOT* r,const char* name);
size_t tarfs_getCountFiles(const uint32_t in);
TarFS_ROOT* tarfs_init(const uint32_t in);

size_t fs_tarfs_read(const char Disk,const char* Path,size_t Offset,size_t Size,void* Buffer);
size_t fs_tarfs_write(const char Disk,const char* Path,size_t Offset,size_t Size,void* Buffer);
FSM_FILE fs_tarfs_info(const char Disk,const char* Path);
int fs_tarfs_create(const char Disk,const char* Path,int Mode);
int fs_tarfs_delete(const char Disk,const char* Path,int Mode);

TarFS_ROOT* fs_tarfs_init(uint32_t in, uint32_t size, int Mode);
FSM_DIR* fs_tarfs_dir(const char Disk,const char* Path);

int fs_tarfs_detect(const char Disk);
void fs_tarfs_label(const char Disk, char* Label);