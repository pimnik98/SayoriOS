#pragma once

#include <common.h>

#define FSM_CHMOD_EXEC               0x01  /// Права выполнения
#define FSM_CHMOD_WRITE              0x02  /// Права записи
#define FSM_CHMOD_READ               0x04  /// Права чтения
#define FSM_CHMOD_SYS                0x08  /// Права системы

typedef struct {
	uint16_t year;	///< Год
	uint8_t month;	///< Месяц
	uint8_t day;	///< День
	uint8_t hour;	///< Час
	uint8_t minute;	///< Минуты
	uint8_t second;	///< Секунды
} __attribute__((packed)) FSM_TIME;

typedef struct {
	int Ready;				///< Существует ли файл?
    char Name[1024];		///< Имя файла
    char Path[1024];		///< Путь файла
    int Mode;				///< Режим файла
    size_t Size;			///< Размер файла в байтах (oсt2bin)
    FSM_TIME LastTime;		///< Время последнего изменения файла
    int Type;				///< Тип элемента
    uint8_t CHMOD;			///< CHMOD файла
} __attribute__((packed)) FSM_FILE;

typedef struct {
	int Ready;				///< Существует ли файл?
	size_t Count;			///< Количество всего
	size_t CountFiles;		///< Количество файлов
	size_t CountDir;		///< Количество папок
	size_t CountOther;			///< Количество неизвестного типа файлов
    FSM_FILE* Files;		///< Файлы и папки
} __attribute__((packed)) FSM_DIR;

///! Буква, Название, откуда, сколько, буфер
typedef size_t (*fsm_cmd_read_t)(const char,const char*,size_t,size_t,void*);

///! Буква, Название, куда, сколько, буфер
typedef size_t (*fsm_cmd_write_t)(const char,const char*,size_t,size_t,void*);

///! Буква, Название
typedef FSM_FILE (*fsm_cmd_info_t)(const char,const char*);

///! Буква, Название
typedef FSM_DIR* (*fsm_cmd_dir_t)(const char,const char*);

///! Буква, Название, Тип (0 - файл | 1 - папка)
typedef int (*fsm_cmd_create_t)(const char,const char*,int);

///! Буква, Название, Тип (0 - файл | 1 - папка)
typedef int (*fsm_cmd_delete_t)(const char,const char*,int);

///! Буква, Буфер
typedef void (*fsm_cmd_label_t)(const char,char*);


///! Буква, Буфер
typedef int (*fsm_cmd_detect_t)(const char);

typedef struct {
	int Ready;					///< Загружена ли фс?
	char Name[64];				///< Наименование драйвера
	int Splash;					///< В какую сторону кинута палка?
	fsm_cmd_read_t Read;		///< Команда для чтения
	fsm_cmd_write_t Write;		///< Команда для записи
	fsm_cmd_info_t Info;		///< Команда для получения информации
	fsm_cmd_dir_t Dir;			///< Команда для получения информации о папке
	fsm_cmd_create_t Create;	///< Команда для создания файла или папка
	fsm_cmd_delete_t Delete;	///< Команда для удаления файла или папка
	
	fsm_cmd_label_t Label;		///< Команда для получения имени диска
	fsm_cmd_detect_t Detect;	///< Команда для определения, предналежит ли диск к фс
	void* Reserved;				///< Можно в ОЗУ дописать доп.данные если требуется.
} __attribute__((packed)) FSM; 


int fsm_getIDbyName(const char* Name);
size_t fsm_read(int FIndex, char DIndex, const char* Name, size_t Offset, size_t Count, void* Buffer);
size_t fsm_write(int FIndex, char DIndex, const char* Name, size_t Offset, size_t Count, void* Buffer);
FSM_FILE fsm_info(int FIndex, char DIndex, const char* Name);
void fsm_reg(const char* Name,int Splash,fsm_cmd_read_t Read, fsm_cmd_write_t Write, fsm_cmd_info_t Info, fsm_cmd_create_t Create, fsm_cmd_delete_t Delete, fsm_cmd_dir_t Dir, fsm_cmd_label_t Label, fsm_cmd_detect_t Detect);
int fsm_delete(int FIndex, char DIndex, const char* Name, int Mode);
int fsm_create(int FIndex, char DIndex, const char* Name, int Mode);
void fsm_dump(FSM_FILE file);
int fsm_getMode(int FIndex);
FSM_DIR* fsm_dir(int FIndex, char DIndex, const char* Name);
void fsm_convertUnix(uint32_t unix_time, FSM_TIME* time);
int fsm_isPathToFile(const char* Path,const char* Name);
char* fsm_timePrintable(FSM_TIME time);
void fsm_dpm_update(char Letter);
size_t fsm_DateConvertToUnix(FSM_TIME time);