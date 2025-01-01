#pragma once

#include <common.h>
#include "fs/fsm.h"

typedef struct {
	bool Ready;				///< Готов к работе?
	char Disk;				///< Индекс диска (который надо подставить)
	char Path[1024];		///< Путь
	bool Online;			///< В сети ли диск?
	char FileSystem[64];	///< Название драйвера на диске
	int DriverFS;			///< Загружен ли драйвер фс?
} __attribute__((packed)) NVFS_DECINFO;

NVFS_DECINFO* nvfs_decode(const char* Name);
size_t nvfs_read(const char* Name, size_t Offset, size_t Count, void* Buffer);
int nvfs_create(const char* Name, int Mode);
int nvfs_delete(const char* Name, int Mode);
size_t nvfs_write(const char* Name, size_t Offset, size_t Count, const void *Buffer);
FSM_FILE nvfs_info(const char* Name);
FSM_DIR* nvfs_dir(const char* Name);
