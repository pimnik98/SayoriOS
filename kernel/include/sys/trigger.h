#pragma once

#define TRIGGER_MAIN_RUNNER		0x0000
#define TRIGGER_KEY_PRESSED		0x0001
#define TRIGGER_MOUSE_MOVE		0x0002
#define TRIGGER_MOUSE_CLICK		0x0003
#define TRIGGER_ELF_START		0x0004
#define TRIGGER_ELF_END			0x0005
#define TRIGGER_ELF_FAILRUN		0x0006
#define TRIGGER_MAIN_RUNNER1	0x1111
#define TRIGGER_MAIN_RUNNER2	0x2222
#define TRIGGER_MAIN_RUNNER3	0x3333
#define TRIGGER_MAIN_RUNNER4	0x4444
#define TRIGGER_MAIN_RUNNER5	0x5555
#define TRIGGER_MAIN_RUNNER6	0x6666
#define TRIGGER_MAIN_RUNNER7	0x7777
#define TRIGGER_MAIN_RUNNER8	0x8888
#define TRIGGER_MAIN_RUNNER9	0x9999


typedef void (*trigger_cmd_t)(void*,void*,void*,void*,void*);

typedef struct trigger
{
	size_t  index;				///< Индекс триггера
	int  type;				    ///< Тип триггера
	bool ready;				    ///< Триггер готов к работе
	bool is_not_delete;			///< Триггер НЕ удален и НЕ свободен
	trigger_cmd_t cmd;		    ///< Команда с 5ю аргументами
} trigger_t;

int RegTrigger(int type, trigger_cmd_t handler);
void CallTrigger(int type, void* data1, void* data2, void* data3, void* data4, void* data5);
void DeleteTrigger(int index);
void OnTrigger(int index);
void OffTrigger(int index);
