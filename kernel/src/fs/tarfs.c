/**
 * @file drv/fs/tarfs.c
 * @author Пиминов Никита (nikita.piminoff@yandex.ru)
 * @brief Файловая система TarFS
 * @version 0.3.5
 * @date 2023-10-14
 * @copyright Copyright SayoriOS Team (c) 2022-2024
*/


#include <io/ports.h>
#include <fs/fsm.h>
#include <fs/tarfs.h>
#include <lib/php/pathinfo.h>
#include <drv/disk/dpm.h>
#include "lib/string.h"
#include "mem/vmm.h"

bool tarfs_debug = false;

int oct2bin(char *str, int size) {
    int n = 0;
    char *c = str;
    while (size-- > 0) {
        n *= 8;
        n += *c - '0';
        c++;
    }
    return n;
}

size_t fs_tarfs_read(const char Disk, const char* Path, size_t Offset, size_t Size, void* Buffer){
    if(tarfs_debug)
        qemu_log("[FSM] [TARFS] [READ] D:%d | N: %s | O:%d | S:%d",Disk,Path,Offset,Size);

    TarFS_ROOT* initrd = (TarFS_ROOT*) dpm_metadata_read(Disk);

    for (int i = 1; i < initrd->Count; i++){
		if (!strcmpn(initrd->Files[i].Name,Path))
			continue;

		return dpm_read(Disk,initrd->Files[i].Addr+Offset, Size, Buffer);
	}

    if (tarfs_debug)
        qemu_log("[FSM] [TARFS] [READ] NO FOUND!!");
	return 0;
}

size_t fs_tarfs_write(const char Disk,const char* Path,size_t Offset,size_t Size,void* Buffer){
	return 0;
}

int fs_tarfs_create(const char Disk,const char* Path,int Mode){
	return 0;
}

int fs_tarfs_delete(const char Disk,const char* Path,int Mode){
	return 0;
}

FSM_FILE fs_tarfs_info(const char Disk,const char* Path){
    if (tarfs_debug) qemu_log("[FSM] [TarFS] [Info]");
//	FSM_FILE *file = kcalloc(sizeof(FSM_FILE), 1);
    FSM_FILE file = {};

	TarFS_ROOT* initrd = (TarFS_ROOT*) dpm_metadata_read(Disk);

	for (int i = 1; i < initrd->Count; i++) {
		//qemu_log("[%d] '%s' != '%s'",strcmp(initrd->Files[i].Name,Path),initrd->Files[i].Name,Path);
		if (!strcmpn(initrd->Files[i].Name,Path)) continue;
//		file->Ready = 1;
		file.Ready = 1;
		char* zpath = pathinfo(initrd->Files[i].Name, PATHINFO_DIRNAME);
		//qemu_log("[%d] zpath: %s",strlen(zpath),zpath);
//        memcpy(file->Path,zpath,strlen(zpath));
//        memcpy(file->Name,initrd->Files[i].Name,strlen(initrd->Files[i].Name));
//        file->Mode = 'r';
//        file->Size = initrd->Files[i].Size;
//        file->Type = initrd->Files[i].Type - 48;


        memcpy(file.Path,zpath,strlen(zpath));
        memcpy(file.Name,initrd->Files[i].Name,strlen(initrd->Files[i].Name));
        file.Mode = 'r';
        file.Size = initrd->Files[i].Size;
        file.Type = initrd->Files[i].Type - 48;

        kfree(zpath);
        //return dpm_read(Disk,initrd->Files[i].Addr+Offset,Size,Buffer);
		break;
	}

//	return file[0];
	return file;
}

void fs_tarfs_label(const char Disk, char* Label){
	memcpy(Label,"ustar",strlen("ustar"));
}

int fs_tarfs_detect(const char Disk){
	char* Buffer = kmalloc(5);

	dpm_read(Disk, 257, 5,Buffer);

	bool isTarFS = ((Buffer[0]  != 0x75 || Buffer[1]  != 0x73 || Buffer[2]  != 0x74 || Buffer[3]  != 0x61 || Buffer[4]  != 0x72)?false:true);
    if (tarfs_debug) qemu_log("[0] = %x",Buffer[0]);
    if (tarfs_debug) qemu_log("[1] = %x",Buffer[1]);
    if (tarfs_debug) qemu_log("[2] = %x",Buffer[2]);
    if (tarfs_debug) qemu_log("[3] = %x",Buffer[3]);
    if (tarfs_debug) qemu_log("[4] = %x",Buffer[4]);
    if (tarfs_debug) qemu_log("[TarFS] is: %d",isTarFS);

	kfree(Buffer);
	return (int)isTarFS;
}

FSM_DIR* fs_tarfs_dir(const char Disk,const char* Path){
    if (tarfs_debug) qemu_log("[FSM] [TarFS] [Info] Disk:%d | Path:%s",Disk,Path);
	FSM_DIR *Dir = kcalloc(sizeof(FSM_DIR), 1);
    TarFS_ROOT* initrd = dpm_metadata_read(Disk); // noalloc
    FSM_FILE *Files = kcalloc(sizeof(FSM_FILE), initrd->Count);

	size_t CA = 0, CF = 0, CD = 0, CO = 0;
	for (int i = 1; i < initrd->Count; i++){
		//////////////////////////
		//// Обращаю внимание, для за путь принимается сейчас R:\\Sayori\\
		//// Если вам искать только файлы, то вот вариант
		// char* zpath = pathinfo(initrd->Files[i].Name, PATHINFO_DIRNAME);
		// qemu_log("[%d] %s",strcmpn(zpath,Path),zpath);
		//// Но мне такой вариант, не подходит, мне нужно еще и папки.
		/////////////////////////
		bool isPassed = fsm_isPathToFile(Path, initrd->Files[i].Name) == 1;

		if (!isPassed)
			continue;
		
		char* zpath = pathinfo(initrd->Files[i].Name, PATHINFO_DIRNAME);

		fsm_convertUnix(atoi(initrd->Files[i].LastTime), &Files[CA].LastTime);

		Files[CA].Mode = 'r';
		Files[CA].Size = initrd->Files[i].Size;
		Files[CA].Type = initrd->Files[i].Type - 48;
		Files[CA].Ready = 1;

		memcpy(Files[CA].Path, zpath, strlen(zpath));

		substr(Files[CA].Name, 
			initrd->Files[i].Name, 
			strlen(Path), 
			strlen(initrd->Files[i].Name) - (Files[CA].Type == 5?1:0) - strlen(Path)
		);

		kfree(zpath);
	
		if (Files[CA].Type == 0) {
			/// Это файл
			CF++;
		} else if (Files[CA].Type == 5){
			/// Это папка
			CD++;
		} else {
			/// Рандомный элемент
			CO++;
		}
		CA++;
	}
	Dir->Ready = 1;
	Dir->Count = CA;
	Dir->CountFiles = CF;
	Dir->CountDir = CD;
	Dir->CountOther = CO;
	Dir->Files = Files;
	
	return Dir;
}

size_t fs_tarfs_countFiles(const uint32_t in){
	size_t count = 0;
	uint8_t* ptr = (uint8_t*)in;

	size_t pos = 0;
	TarFS_Elem* elem = (TarFS_Elem*)ptr;

    while (memcmp(elem->Signature, "ustar", 5) == 0) {
        int filesize = oct2bin(elem->Size, 11);

        count++;

		pos = ALIGN(pos + sizeof(TarFS_Elem) + filesize, 512);
        elem = (TarFS_Elem*)(ptr + pos);
    }

	return count;
}

TarFS_ROOT* fs_tarfs_init(uint32_t in, uint32_t size, int Mode){
	uint8_t *ptr = (uint8_t*)in;
	if(ptr[257] != 0x75
	|| ptr[258] != 0x73
	|| ptr[259] != 0x74
	|| ptr[260] != 0x61
	|| ptr[261] != 0x72) {
		return kcalloc(sizeof(TarFS_ROOT), 1);
	}

	size_t count = fs_tarfs_countFiles(in);

    if (tarfs_debug) qemu_log("Found %d entries.", count);

	size_t currentInx = 0;
	ssize_t sizeDir = -1;

	TarFS_File* tffs = kcalloc(count + 1, sizeof(TarFS_File));
	TarFS_ROOT* root = kcalloc(sizeof(TarFS_ROOT), 1);

	size_t pos = 0;

	TarFS_Elem *file = (TarFS_Elem*) ptr;

	while (memcmp(file->Signature, "ustar", 5) == 0) {
		int filesize = oct2bin(file->Size, 11);

		if (sizeDir == -1 && Mode == 1){
			//qemu_log("Mode:1 | s:%d | m:%d",sizeDir,Mode);
			sizeDir = strlen(file->Name);
		} else if (sizeDir == -1) {
			//qemu_log("Mode:0");
			sizeDir = 2;
		}

		tffs[currentInx].Ready = (currentInx==0?0:1);
		tffs[currentInx].Size = filesize;
		tffs[currentInx].Type = (int)file->Type;
		tffs[currentInx].Real = (uint32_t)(ptr + pos) + sizeof(TarFS_Elem);
		tffs[currentInx].Addr = tffs[currentInx].Real - in;


		memcpy(tffs[currentInx].Mode,file->Mode, 8);
		memcpy(tffs[currentInx].LastTime,file->LastTime,12);
		substr(tffs[currentInx].Name,file->Name,sizeDir,100-sizeDir);
		//qemu_log("[%d] fix name:%s",strlen(tffs[currentInx].Name),tffs[currentInx].Name);
		currentInx++;

        if (tarfs_debug) qemu_log("[%x] Name: %s; Size: %d", file, file->Name, filesize);

		pos = ALIGN(pos + sizeof(TarFS_Elem) + filesize, 512);
		file = (TarFS_Elem*)(ptr + pos);
		//        ptr += (((filesize + 511) / 512) + 1) * 512;
    }

//	while(1);
	
	root->Ready = 1;
	root->Count = count;
	root->Files = tffs;

	return root;
}

TarFS_File tarfs_infoFile(TarFS_ROOT* r,const char* name){
	for (int i = 1;i < r->Count;i++){
		if (!strcmpn(r->Files[i].Name,name)) continue;
		return r->Files[i];
	}
	return r->Files[0];
}

char* tarfs_readFile(TarFS_ROOT* r, const char* name){
	for (int i = 1; i < r->Count; i++) {
		if (!strcmpn(r->Files[i].Name,name))
			continue;

		char* buffer = kmalloc(r->Files[i].Size);

		memcpy(buffer, (char*)r->Files[i].Real, r->Files[i].Size);

		return buffer;
	}
	return 0;
}

