#pragma once

#include <common.h>

#define SECTOR_SIZE 512
#define FAT_DELETED_MAGIC ((char)0xE5)
#define ENTRY_AVAILABLE 0x00
#define ENTRY_ERASED 0xe5
#define offsetof(t,d) __builtin_offsetof(t, d)
typedef uint32_t addr_t;
typedef struct {
    uint8_t  jump_code[3];				///< Первые три байта EB 3C 90 дизассемблируются в JMP SHORT 3C NOP. (Значение 3C может быть другим.) 
    char     oem_name[8];				///< ОЕМ-идентификатор.
    uint16_t bytes_per_sector;			///< Количество байтов на сектор 
    uint8_t  sectors_per_cluster;		///< Количество секторов в кластере.
    uint16_t reserved_sectors;			///< Количество зарезервированных секторов. Сектора загрузочной записи включены в это значение.
    uint8_t  fat_count;					///< Количество таблиц размещения файлов (FAT) на носителе. Часто это значение равно 2.
    uint16_t root_dir_capacity;			///< Количество записей корневого каталога (должно быть установлено так, чтобы корневой каталог занимал целые сектора).
    uint16_t logical_sectors16;			///< Общее количество секторов в логическом томе. Если это значение равно 0, это означает, что в томе более 65535 секторов, а фактическое количество сохраняется в записи «Счетчик больших секторов» по ​​адресу 0x20.
    uint8_t  media_type;				///< Этот байт указывает тип дескриптора носителя .
    uint16_t sectors_per_fat;			///< Количество секторов на FAT. Только FAT12/FAT16.
    uint16_t chs_sectors_per_track;		///< Количество секторов на дорожке.
    uint16_t chs_tracks_per_cylinder;	///< Количество головок или сторон на носителе.
    uint32_t hidden_sectors;			///< Количество скрытых секторов. (т.е. LBA начала раздела.)
    uint32_t logical_sectors32;			///< Большое количество секторов. Это поле устанавливается, если в томе более 65535 секторов, что приводит к значению, которое не соответствует записи числа секторов в 0x13.
    uint8_t  media_id;					///< Номер привода. Значение здесь должно быть идентично значению, возвращаемому прерыванием BIOS 0x13 или переданному в регистр DL; т.е. 0x00 для гибкого диска и 0x80 для жестких дисков.
    uint8_t  chs_head;					///< Флаги в Windows NT. В противном случае зарезервировано.
    uint8_t  ext_bpb_signature;			///< Подпись (должна быть 0x28 или 0x29).
    uint32_t serial_number;				///< VolumeID 'Серийный' номер. Используется для отслеживания томов между компьютерами. Вы можете игнорировать это, если хотите.
    char     volume_label[11];		///< Метка
    char     fsid[8];				///< Строка идентификатора системы. Это поле является строковым представлением типа файловой системы FAT. Он заполнен пробелами. Спецификация говорит, что никогда нельзя доверять содержимому этой строки для любого использования.
    uint8_t  boot_code[448];		///< Загрузочный код.
    uint16_t magic;					///< Подпись загрузочного раздела 0xAA55.
} __attribute__((__packed__)) FAT_BOOT_SECTOR;


typedef struct{
    uint8_t READONLY: 1,
    HIDDEN: 1,
    SYSTEN: 1,
    VOLUMEID: 1,
    DIR: 1,
    ARCHIVE: 1,
    _reserved: 2;
}__attribute__((__packed__)) fat_attributes;


typedef struct{
    char Name[11];						///< 8.3 имя файла. Первые 8 символов — это имя, а последние 3 — расширение.
    fat_attributes Attr;				///< Атрибуты файла
    uint8_t Reser;						///< Зарезервировано для использования Windows NT.
    uint8_t TimeMS;						///< Время создания в десятых долях секунды.
    uint16_t TimeCreateHIS;				///< Время создания файла. Умножьте секунды на 2. (Часы 5 / Минуты 6 / Секунды 5)
    uint16_t TimeCreateDate;			///< Дата создания файла (Год 7 / Месяц 4 / День 5)
    uint16_t TimeAccess;				///< Дата последнего доступа. Тот же формат, что и дата создания.
    uint16_t FCHB;						///< Старшие 16 бит первого номера кластера этой записи. Для FAT 12 и FAT 16 это значение всегда равно нулю.
    uint16_t TimeEditHIS;				///< Время последней модификации. Тот же формат, что и время создания.
    uint16_t TimeEditDate;				///< Дата последней модификации. Тот же формат, что и дата создания.
    uint16_t FCLB;						///< Младшие 16 бит первого номера кластера этой записи. Используйте этот номер, чтобы найти первый кластер для этой записи.
    uint32_t Size;						///< Размер файла в байтах.
} FAT_ENTRY;

struct dir_entry_t{
    char name[13];
    uint32_t size;
    uint8_t is_archived: 1;
    uint8_t is_readonly: 1;
    uint8_t is_system: 1;
    uint8_t is_hidden: 1;
    uint8_t is_directory: 1;
};

struct disk_t{
    FILE* pdisk;
    int32_t sec_count;
};

struct volume_t {
    FAT_BOOT_SECTOR super;
    uint32_t fat1_pos;
    uint32_t root_pos;
    uint32_t root_sectors;
    uint32_t cluster2_pos;
    struct disk_t* disk;
    uint32_t* fat_table;
};

struct file_t{
    FAT_ENTRY *entry;
    struct volume_t* volume;
    uint32_t offset;
};
struct dir_t{
    FAT_ENTRY * entries;
    uint32_t capactiy;
    uint32_t offset;
    struct volume_t* volume;
};