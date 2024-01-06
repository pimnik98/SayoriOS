/**
 * @file drv/disk/mbr.c
 * @author Пиминов Никита (nikita.piminoff@yandex.ru)
 * @brief MBR Info
 * @version 0.3.4
 * @date 2023-08-03
 * @copyright Copyright SayoriOS Team (c) 2022-2023
*/

#include "common.h"
#include "io/tty.h"
#include <io/ports.h>

typedef struct {
	uint8_t Attr;			///< Атрибуты диска (7 = активный)
	uint8_t StartCHS[3];	///< CHS Адрес начала раздела
	uint8_t Type;			///< Тип раздела
	uint8_t EndCHS[3];		///< CHS Адрес последнего сектора раздела
	uint32_t LBA;			///< Начало раздела LBA
	uint32_t Size;			///< Количество секторов
} __attribute__((packed)) MBR_PARTITION_INFO; 


typedef struct {
	char Bootstrap[440];		///< Загрузщик
	uint8_t UniqueDiskID[4];	///< Уникальный индентификатор диска
	uint8_t Reserved[2];		///< Зарезервировано 0x0000 | 0x5a5a - ReadOnly
	MBR_PARTITION_INFO Part0;	///< Первая запись в таблице разделов
	MBR_PARTITION_INFO Part1;	///< Вторая запись в таблице разделов
	MBR_PARTITION_INFO Part2;	///< Третья запись в таблице разделов
	MBR_PARTITION_INFO Part3;	///< Четвертая запись в таблице разделов
	uint16_t Sign;			///< Подпись
} MBR_INFO; 

void _mbr_info(){
	// Выделяем место и копируем байты
	MBR_INFO *mbr = kcalloc(sizeof(MBR_INFO), 1);

	ata_read(0, mbr, 0, sizeof(MBR_INFO));
	tty_printf("[MBR] Sign:  %x\n", mbr[0].Sign);

	//memcpy(mbr, addr, sizeof(MBR_INFO));
	// Выводим инфу
	//int x = (mbr[0].UniqueDiskID[2] « 16) | (mbr[0].UniqueDiskID[1] « 8) | mbr[0].UniqueDiskID[0];
	tty_printf("[MBR] UniqueDiskID: %d-%d-%d-%d\n",
				mbr[0].UniqueDiskID[0],
				mbr[0].UniqueDiskID[1],
				mbr[0].UniqueDiskID[2],
				mbr[0].UniqueDiskID[3]
	);

	//tty_printf("[MBR] UniqueDiskID: %d | %x\n", x, x);
	tty_printf("[MBR] Part0: Attr: %x\n",mbr[0].Part0.Attr);
	tty_printf("[MBR] Part0: Type: %x\n",mbr[0].Part0.Type);
	tty_printf("[MBR] Part0: Size: %x\n",mbr[0].Part0.Size);


	tty_printf("[MBR] Part1: Attr: %x\n",mbr[0].Part1.Attr);
	tty_printf("[MBR] Part1: Type: %x\n",mbr[0].Part1.Type);
	tty_printf("[MBR] Part1: Size: %x\n",mbr[0].Part1.Size);


	tty_printf("[MBR] Part2: Attr: %x\n",mbr[0].Part2.Attr);
	tty_printf("[MBR] Part2: Type: %x\n",mbr[0].Part2.Type);
	tty_printf("[MBR] Part2: Size: %x\n",mbr[0].Part2.Size);


	tty_printf("[MBR] Part3: Attr: %x\n",mbr[0].Part3.Attr);
	tty_printf("[MBR] Part3: Type: %x\n",mbr[0].Part3.Type);
	tty_printf("[MBR] Part3: Size: %x\n",mbr[0].Part3.Size);
}