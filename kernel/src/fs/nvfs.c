/**
 * @file drv/fs/nvfs.c
 * @author Пиминов Никита (nikita.piminoff@yandex.ru)
 * @brief New Virtual File System - Новая виртуальная файловая система
 * @version 0.3.4
 * @date 2023-10-14
 * @warning Некит, напиши документацию
 * @copyright Copyright SayoriOS Team (c) 2022-2023
*/

#include <io/ports.h>  
#include <drv/disk/dpm.h> 
#include <fs/nvfs.h>

#include "../lib/libstring/include/string.h"
#include "mem/vmm.h"
#include "lib/php/explode.h"
#include "fs/fsm.h"
#include "lib/php/str_replace.h"

bool nvfs_debug = false;



NVFS_DECINFO nvfs_decode(const char* Name){
	NVFS_DECINFO info = {};
	info.Ready = 0;
	info.DriverFS = -1;
	uint32_t pc = str_cdsp2(Name, ':');

	string_t* strname = string_from_charptr(Name);
	vector_t* out = string_split(strname, ":");

	if (pc != 1)
		goto end;

	info.Disk = ADDR2STRING(out->data[0])->data[0];

	substr(info.Path, ADDR2STRING(out->data[1])->data, 1 , ADDR2STRING(out->data[1])->length - 1);

	DPM_Disk disk = dpm_info(info.Disk);

	if (disk.Ready != 1)
		goto end;

	info.Online = 1;
	memcpy(info.FileSystem, disk.FileSystem, sizeof(disk.FileSystem));

	info.DriverFS = fsm_getIDbyName(info.FileSystem);

	if (info.DriverFS == -1)
		goto end;

	int fgm = fsm_getMode(info.DriverFS);

	if (fgm == 0){
		char_replace(0x2F,0x5C,info.Path);
	} else {
		char_replace(0x5C,0x2F,info.Path);
	}

	info.Ready = 1;

	end:

    string_split_free(out);
    string_destroy(strname);

	return info;
}

size_t nvfs_read(const char* Name, size_t Offset, size_t Count, void* Buffer){
	const NVFS_DECINFO vinfo = nvfs_decode(Name);
	if (vinfo.Ready == 0) return 0;
	return fsm_read(vinfo.DriverFS, vinfo.Disk, vinfo.Path, Offset, Count, Buffer);
}

int nvfs_create(const char* Name, int Mode){
	const NVFS_DECINFO vinfo = nvfs_decode(Name);
	if (vinfo.Ready == 0) return 0;
	return fsm_create(vinfo.DriverFS, vinfo.Disk, vinfo.Path, Mode);
}

int nvfs_delete(const char* Name, int Mode){
	const NVFS_DECINFO vinfo = nvfs_decode(Name);
	if (vinfo.Ready == 0)
		return 0;
	return fsm_delete(vinfo.DriverFS, vinfo.Disk, vinfo.Path, Mode);
}

size_t nvfs_write(const char* Name, size_t Offset, size_t Count, const void *Buffer){
	const NVFS_DECINFO vinfo = nvfs_decode(Name);
	if (vinfo.Ready == 0)
		return 0;
	return fsm_write(vinfo.DriverFS, vinfo.Disk, vinfo.Path, Offset, Count, Buffer);
}

FSM_FILE nvfs_info(const char* Name){
	NVFS_DECINFO vinfo = nvfs_decode(Name);  // no memleak
    if (nvfs_debug) qemu_log("NVFS INFO:\nReady: %d\nDisk: [%d] %c\nPath: [%d]  %s\nDisk Online: %d\nDisk file system: [%d] %s\nLoaded in file system driver: %d",vinfo.Ready,vinfo.Disk,vinfo.Disk,strlen(vinfo.Path),vinfo.Path,vinfo.Online,strlen(vinfo.FileSystem),vinfo.FileSystem,vinfo.DriverFS);

	if (vinfo.Ready != 1){
		return (FSM_FILE){};
	}

	FSM_FILE file = fsm_info(vinfo.DriverFS, vinfo.Disk, vinfo.Path);
	
	return file;
}

FSM_DIR* nvfs_dir(const char* Name){
	NVFS_DECINFO vinfo = nvfs_decode(Name);

	if (vinfo.Ready != 1) {
		FSM_DIR* dir = kcalloc(sizeof(FSM_DIR), 1);
		return dir;
	}

	FSM_DIR* dir = fsm_dir(vinfo.DriverFS, vinfo.Disk, vinfo.Path);
	return dir;
}

/*

void vnfs_test(){

	FSM_FILE file = nvfs_info("R:\\help next you\\main.c");
	//qemu_log("Ready: %d",file.Ready);
	fsm_dump(file);

	char* bf1 = kmalloc(file.Size);
	int rf1 = nvfs_read("R:\\help next you\\main.c",10,5,bf1);

	qemu_log("Read %d / %d |\n%s\n",rf1,file.Size,bf1);

	FSM_FILE file2 = nvfs_info("b:\\pizdec\\вот это драйвер\\ахуеть.exe");

	fsm_dump(file2);
	while(1){}
}*/
