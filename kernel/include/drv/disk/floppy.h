#pragma once

#include <common.h>
#define floppy_dmalen 0x4800
#define FLOPPY_MAIN 0x03f0
#define FLOPPY_SECO 0x0370
#define FLOPPY_144_CODE 4
#define FLOPPY_144_SECTORS_PER_TRACK 18
#define FLOPPY_144_SECTOR_SIZE 512
#define FLOPPY_144_CYLINDER_FACE_SIZE (FLOPPY_144_SECTORS_PER_TRACK * FLOPPY_144_SECTOR_SIZE)
#define FLOPPY_144_HEADS 2
#define FLOPPY_144_CYLINDER_SIZE (FLOPPY_144_CYLINDER_FACE_SIZE * FLOPPY_144_HEADS)

#define FDA 0	///< Floppy Device A
#define FDB 1	///< Floppy Device B

enum FloppyErrors{
	FLOPPY_ERROR_NOSUPPORT		=	1,	///< Устройство не поддерживается
	FLOPPY_ERROR_DMA			=	2,	///< Ошибка при работе с буфером DMA
	FLOPPY_ERROR_MOTOR			=	3,	///< Мотор ведет себя странно
	FLOPPY_ERROR_CMD			=	4,	///< Ошибка отправки команды на устройство
	FLOPPY_ERROR_READ			=	5,	///< Ошибка считывания ответа
	FLOPPY_ERROR_CALIBRATE		=	6,	///< Ошибка калибровки устройства
	FLOPPY_ERROR_TRACK			=	7,	///< Ошибка перехода на требуемую дорожку
	FLOPPY_ERROR_EOL			=	8,	///< Конец дорожки
	FLOPPY_ERROR_NOREADY		=	9,	///< Устройство не готово
	FLOPPY_ERROR_CRC			=	10,	///< Ошибка считывания CRC
	FLOPPY_ERROR_CONTROLLER		=	11,	///< Контроллер не отвечает
	FLOPPY_ERROR_NODATA			=	12,	///< Информация не найдена
	FLOPPY_ERROR_NOADDR			=	13,	///< Адресная метка не найдена
	FLOPPY_ERROR_DELADDR		=	14,	///< Адресная метка была удалена
	FLOPPY_ERROR_CYLINDER		=	15,	///< Ошибка работы цилиндера
	FLOPPY_ERROR_UPD765			=	16,	///< Сектор uPD765 не найден
	FLOPPY_ERROR_512B			=	17,	///< Требование в 512 байт на цилиндер
	FLOPPY_ERROR_WRITE			=	18,	///< Ошибка записи данных
	FLOPPY_ERROR_CMD_UNK		=	19,	///< Неизвестная команда
	FLOPPY_ERROR_20INV			=	20,	///< Истрачены все 20 попыток, для настройки устройства
	FLOPPY_ERROR_SEEK			=	21,	///< Ошибка назначения позиции
	FLOPPY_ERROR_INVTRACK		=	22,	///< Невалидный трек
	FLOPPY_ERROR_RESET			=	23,	///< Ошибка сброса устройства
};

enum FloppyCommands
{
   FLOPPY_CMD_READ_TRACK =                 2,	///> Считывает трек, и генерирует IRQ6
   FLOPPY_CMD_SPECIFY =                    3,	///> [Исп] Установка параметров устройства
   FLOPPY_CMD_SENSE_DRIVE_STATUS =         4,	///> Получение статуса устройства
   FLOPPY_CMD_WRITE_DATA =                 5,	///> [Исп] Запись данных на устройство
   FLOPPY_CMD_READ_DATA =                  6,	///> [Исп] Чтение данных с устройства
   FLOPPY_CMD_RECALIBRATE =                7,	///> [Исп] Сброс состояния устройства, и переход в начало
   FLOPPY_CMD_SENSE_INTERRUPT =            8,	///> [Исп] Опрос IRQ6 и получение статуса последней команды
   FLOPPY_CMD_WRITE_DELETED_DATA =         9,	///> (?) Запись удаленных данных
   FLOPPY_CMD_READ_ID =                    10,	///> (?) Считывание по  ID и генерация IRQ6
   FLOPPY_CMD_READ_DELETED_DATA =          12,	///> (?) Считывание удаленных данных
   FLOPPY_CMD_FORMAT_TRACK =               13,	///> (?) Форматирование дорожки
   FLOPPY_CMD_DUMPREG =                    14,	///> (?)
   FLOPPY_CMD_SEEK =                       15,	///> [Исп] Перемещение головки на определенную головку | цилиндер
   FLOPPY_CMD_VERSION =                    16,	///> Используется во время инициализации, один раз
   FLOPPY_CMD_SCAN_EQUAL =                 17,	///> (?) 
   FLOPPY_CMD_PERPENDICULAR_MODE =         18,	///> Используется во время инициализации, один раз
   FLOPPY_CMD_CONFIGURE =                  19,	///> Установка настроек устройства
   FLOPPY_CMD_LOCK =                       20,	///> Установка защиты настроек от сброса
   FLOPPY_CMD_VERIFY =                     22,	///> (?) Проверка
   FLOPPY_CMD_SCAN_LOW_OR_EQUAL =          25,	///> (?)
   FLOPPY_CMD_SCAN_HIGH_OR_EQUAL =         29	///> (?)
};

enum { 
	floppy_motor_off = 0,	///< Мотор выключен
	floppy_motor_on,		///< Мотор включен
	floppy_motor_wait		///< Мотор ожидает команды
};

typedef enum {
    FLOPPY_READ = 1,	///< Режим чтения
    FLOPPY_WRITE = 2	///< Режим записи
} FloppyMode;

enum floppy_registers {
   FLOPPY_DOR  = 2,  // digital output register
   FLOPPY_MSR  = 4,  // master status register, read only
   FLOPPY_FIFO = 5,  // data FIFO, in DMA operation for commands
   FLOPPY_CCR  = 7   // configuration control register, write only
};

// The commands of interest. There are more, but we only use these here.
enum floppy_commands {
   CMD_SPECIFY = 3,            // SPECIFY
   CMD_WRITE_DATA = 5,         // WRITE DATA
   CMD_READ_DATA = 6,          // READ DATA
   CMD_RECALIBRATE = 7,        // RECALIBRATE
   CMD_SENSE_INTERRUPT = 8,    // SENSE INTERRUPT
   CMD_SEEK = 15,              // SEEK
};

enum FloppyRegisters
{
   FLOPPY_STATUS_REGISTER_A                = 0,	///< Статус устройства A|1, в режиме только чтения
   FLOPPY_STATUS_REGISTER_B                = 1,	///< Статус устройства B|2, в режиме только чтения
   FLOPPY_DIGITAL_OUTPUT_REGISTER          = 2,	///< [Исп] Цифровой выходной регистр (DOR)
   FLOPPY_TAPE_DRIVE_REGISTER              = 3,	///< (?) 
   FLOPPY_MAIN_STATUS_REGISTER             = 4,	///< [Исп] Статус устройства, в режиме только чтения
   FLOPPY_DATARATE_SELECT_REGISTER         = 4,	///< (?) Доступно в режиме только записи
   FLOPPY_DATA_FIFO                        = 5,	///< [Исп] Данные FIFO, для доступа через DMA (прямого доступа к памяти)
   FLOPPY_DIGITAL_INPUT_REGISTER           = 7,	///< (?) Доступно в режиме только чтения
   FLOPPY_CONFIGURATION_CONTROL_REGISTER   = 7	///< [Исп] Регистр для управления конфигурацией, в режиме только записи
};
typedef int (*FloppyRW_t)(int,char*,uint32_t,uint32_t);
typedef int (*FloppyCache_t)(int,FloppyMode,uint32_t,uint32_t*,uint32_t*);
typedef struct floppy
{
	int  Index;				///< Индекс устройства
	int  Addr;				///< Адрес устройства
	int  Status;			///< Статус
	int  LastErr;			///< Посл. ошибка
	int  Ticks;				///< Тики работы дисковода
	int  Type;				///< Тип устройства
	int  Motor;				///< Статус мотора
	char Name[12];			///< Название дискеты
	char FileSystem[12];	///< Файловая система
	unsigned Cyr;			///< ?
	FloppyRW_t Read;		///< Команда для чтения данных
	FloppyRW_t Write;		///< Команда для записи данных
	FloppyCache_t Cache;	///< Команда для кэша
	const char* Buffer;		///< Буфер
} floppy_t;