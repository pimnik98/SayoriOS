/**
 * @file drv/fs/nvfs.c
 * @author Пиминов Никита (nikita.piminoff@yandex.ru)
 * @brief New Virtual File System - Новая виртуальная файловая система
 * @version 0.3.5
 * @date 2023-10-14
 * @warning Некит, напиши документацию
 * @copyright Copyright SayoriOS Team (c) 2022-2024
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

NVFS_DECINFO* nvfs_decode(const char* Name) {
	NVFS_DECINFO* info = kcalloc(sizeof(NVFS_DECINFO), 1);
	
	info->DriverFS = -1;
	
	qemu_log("Decoding name: %s (%x)", Name, Name);

	// Is path header valid?
	bool is_valid_delim = struntil(Name, ':') == 1 && (struntil(Name, '\\') == 2 || struntil(Name, '/') == 2);

	if (!is_valid_delim) {
		goto end;
	}

	// Disk is always first letter of the path.
	info->Disk = Name[0];

	qemu_log("Disk: %d", info->Disk);


	// Now cut a rest of path with trailing \ (or /)
	substr(info->Path, Name, 2, strlen(Name + 2));

	qemu_log("Supposed path: %s", info->Path);


	// Get disk info.
	DPM_Disk disk = dpm_info(info->Disk);

	if (disk.Ready != 1) {
		goto end;
	}

	info->Online = 1;
	memcpy(info->FileSystem, disk.FileSystem, sizeof(disk.FileSystem));

	info->DriverFS = fsm_getIDbyName(info->FileSystem);

	if (info->DriverFS == -1) {
		goto end;
	}

	int fgm = fsm_getMode(info->DriverFS);

	if (fgm == 0){
		char_replace(0x2F,0x5C,info->Path);
	} else {
		char_replace(0x5C,0x2F,info->Path);
	}

	info->Ready = 1;

	end:

	return info;
}

size_t nvfs_read(const char* Name, size_t Offset, size_t Count, void* Buffer){
	if(nvfs_debug) {
		qemu_log("Name=%s", Name);
	}

	NVFS_DECINFO* vinfo = nvfs_decode(Name);
	size_t res = 0;

	if (vinfo->Ready == 0) {
		goto end;
	}
	
	res = fsm_read(vinfo->DriverFS, vinfo->Disk, vinfo->Path, Offset, Count, Buffer);

end:
	kfree(vinfo);

	return res;
}

int nvfs_create(const char* Name, int Mode){
	NVFS_DECINFO* vinfo = nvfs_decode(Name);
	size_t res = 0;

	if (vinfo->Ready == 0) {
		goto end;
	}

	res = fsm_create(vinfo->DriverFS, vinfo->Disk, vinfo->Path, Mode);

end:
	kfree(vinfo);
	return res;
}

int nvfs_delete(const char* Name, int Mode){
	NVFS_DECINFO* vinfo = nvfs_decode(Name);
	size_t res = 0;

	if (vinfo->Ready == 0) {
		goto end;
	}
	
	res = fsm_delete(vinfo->DriverFS, vinfo->Disk, vinfo->Path, Mode);

	end:

	kfree(vinfo);

	return res;
}

size_t nvfs_write(const char* Name, size_t Offset, size_t Count, const void *Buffer){
	NVFS_DECINFO* vinfo = nvfs_decode(Name);
	size_t res = 0;

	if (vinfo->Ready == 0) {
		goto end;
	}

	res = fsm_write(vinfo->DriverFS, vinfo->Disk, vinfo->Path, Offset, Count, Buffer);

	end:

	kfree(vinfo);

	return res;
}

FSM_FILE nvfs_info(const char* Name){
	NVFS_DECINFO* vinfo = nvfs_decode(Name);  // no memleak
    if (nvfs_debug) {
	    qemu_log("NVFS INFO:\n"
		     "Ready: %d\n"
		     "Disk: [%d] %c\n"
		     "Path: [%d]  %s\n"
		     "Disk Online: %d\n"
		     "Disk file system: [%d] %s\n"
		     "Loaded in file system driver: %d",
		     vinfo->Ready,
		     vinfo->Disk,
		     vinfo->Disk,
		     strlen(vinfo->Path),
		     vinfo->Path,
		     vinfo->Online,
		     strlen(vinfo->FileSystem),
		     vinfo->FileSystem,
		     vinfo->DriverFS
		);
    }

	FSM_FILE file = {};

	if (vinfo->Ready != 1){
		goto end;
	}

	file = fsm_info(vinfo->DriverFS, vinfo->Disk, vinfo->Path);
end:

	kfree(vinfo);

	return file;
}

FSM_DIR* nvfs_dir(const char* Name){
	NVFS_DECINFO* vinfo = nvfs_decode(Name);

	if (vinfo->Ready != 1) {
		kfree(vinfo);
		FSM_DIR* dir = kcalloc(sizeof(FSM_DIR), 1);
		return dir;
	}

	FSM_DIR* dir = fsm_dir(vinfo->DriverFS, vinfo->Disk, vinfo->Path);
	
	kfree(vinfo);
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
