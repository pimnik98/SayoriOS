/**
 * @file drv/fs/smfs.c
 * @author Пиминов Никита (nikita.piminoff@yandex.ru)
 * @brief Файловая система SMFS
 * @version 0.3.4
 * @date 2023-10-14
 * @copyright Copyright SayoriOS Team (c) 2022-2023
*/

#include <kernel.h>
#include <io/ports.h> 
#include <fs/fsm.h>
#include <fs/smfs.h>
#include <lib/php/pathinfo.h>


size_t fs_smfs_read(const char Disk,const char* Path, size_t Offset, size_t Size,void* Buffer){
	return 0;
}

size_t fs_smfs_write(const char Disk,const char* Path,size_t Offset,size_t Size,void* Buffer){
	return 0;
}

FSM_FILE fs_smfs_info(const char Disk,const char* Path){
	
}

FSM_DIR* fs_smfs_dir(const char Disk,const char* Path){
	
}

int fs_smfs_create(const char Disk,const char* Path,int Mode){
	return 0;
}

int fs_smfs_delete(const char Disk,const char* Path,int Mode){
	return 0;
}

void fs_smfs_label(const char Disk, char* Label){
	memcpy(Label,"SMFS",strlen("SMFS"));
}

int fs_smfs_format(const char Disk){

	DPM_Disk IDisk = dpm_info(Disk);
	size_t MaxFiles = (((IDisk.Size) - (sizeof(SMFS_BOOT_SECTOR))) / 2048);	/// 1024 ф.
	if (MaxFiles <= 0){
		qemu_log("FATAL FORMAT!");
		return 0;
	}
	qemu_log("FORMAT IS STARTING...");
	
	size_t read = 0,seek = 0;
	int sizeFiles = ((sizeof(SMFS_Elements)) * MaxFiles); ////< 74 * 1024 = 
	int freeSpa = ((IDisk.Size) - (sizeFiles) - (sizeof(SMFS_BOOT_SECTOR)));
	int allPkg = (freeSpa / (sizeof(SMFS_PACKAGE)));
	
	qemu_log("[SMFS] Formatting FDA STARTED!");
	qemu_log("[SMFS] DISK_SIZE = %d", (IDisk.Size));	///< 2097152
	qemu_log("[SMFS] BOOT_SIZE = %d", (sizeof(SMFS_BOOT_SECTOR))); ///< 39
	qemu_log("[SMFS] Max Elems = %d | %d", MaxFiles, sizeFiles); ///< 1024 | 75776
	qemu_log("[SMFS] FREE SPA = %d", freeSpa); ///< (2021337)
	qemu_log("[SMFS] Max Package = %d", allPkg);	///< 2021337 / 15 = мин. 134756 | макс. 1078048 байт 
	qemu_log("[SMFS] We overwrite on our own BOOT SECTOR");
	
	char oem[8] = {'S','A','Y','O','R','I','O','S'};
	char label[11] = {'S','M','F','S',' ',' ',' ',' ',' ',' ',0};
	char fsid[8] = {'S','M','F','S','1','.','0',0};
	
	SMFS_BOOT_SECTOR *boot = kmalloc(sizeof(SMFS_BOOT_SECTOR));

	boot[0].magic1 = 0x7246;
	boot[0].magic2 = 0xCAFE;
	boot[0].MaximumElems = MaxFiles;
	boot[0].MaxPackage = allPkg;
	
	memcpy((void*) boot[0].oem_name, oem, 8);
	memcpy((void*) boot[0].volume_label, label, 11);
	memcpy((void*) boot[0].fsid, fsid, 8);

	read = dpm_write(Disk, seek, sizeof(SMFS_BOOT_SECTOR), &boot[0]);

	//read = _FloppyWrite(0,&boot[0],0,sizeof(SMFS_BOOT_SECTOR));
	
	qemu_log("[SMFS] FDA WRITE:%d\n",read);
	
	if (read == -1){
		qemu_log("[SMFS] FATAL FORMAT!");
		return 0;
	}
	
	seek = sizeof(SMFS_BOOT_SECTOR);
	qemu_log("[SMFS] Create markup structures for files and folders");
	
	SMFS_Elements *elements = kmalloc(sizeof(SMFS_Elements));

	char eName[32] = {0};
	elements[0].Attr = SMFS_TYPE_DELETE;
	elements[0].Size = 0;
	elements[0].TimeCreateHIS = 0;
	elements[0].TimeCreateDate = 0;
	elements[0].TimeAccess = 0;
	elements[0].Dir = 0;
	elements[0].Point = 0;
	
	memcpy((void*) elements[0].Name, eName, 32);
	
	for (int i = 0; i < MaxFiles; i++) {
		elements[0].Index = i;
		
		read = dpm_write(Disk, seek, sizeof(SMFS_Elements), &elements[0]);
		//read = _FloppyWrite(0,&elements[0],seek,sizeof(SMFS_Elements));
		qemu_log("[SMFS] DISK WRITE:%d | I: %d | In: %x\n",read,i,seek);
		seek += sizeof(SMFS_Elements);
	}
	
	qemu_log("[SMFS] Now we write packages");
	
	SMFS_PACKAGE *pkg = kmalloc(sizeof(SMFS_PACKAGE));
	char eData[8] = {0};
	memcpy((void*) pkg[0].Data, eData, 8);
	pkg[0].Length = 0;
	pkg[0].Next = -1;
	pkg[0].Status = SMFS_PACKAGE_FREE;
	
	for (int i = 0; i < allPkg; i++){
		
		read = dpm_write(Disk, seek, sizeof(SMFS_PACKAGE), &pkg[0]);

		//read = _FloppyWrite(0,&pkg[0],seek,sizeof(SMFS_PACKAGE));
		qemu_log("[SMFS] DISK WRITE:%d | I: %d | In: %x\n",read,i,seek);
		seek += sizeof(SMFS_PACKAGE);
	}
	return 1;
}

int fs_smfs_detect(const char Disk){
	SMFS_BOOT_SECTOR* BOOT = kmalloc(sizeof(SMFS_BOOT_SECTOR)+1);
	dpm_read(Disk, 0, sizeof(SMFS_BOOT_SECTOR), BOOT);

	
	qemu_log("[SMFS] Check BootSector:");
	qemu_log(" |--- magic1       | %x\n",BOOT->magic1);
	qemu_log(" |--- magic2       | %x\n",BOOT->magic2);
	qemu_log(" |--- MaximumElems | %d\n",BOOT->MaximumElems);
	qemu_log(" |--- MaxPackage   | %d\n",BOOT->MaxPackage);
	qemu_log(" |--- OEM          | %s\n",BOOT->oem_name);
	qemu_log(" |--- Label        | %s\n",BOOT->volume_label);
	qemu_log(" |--- FSID         | %s\n",BOOT->fsid);

	kfree(BOOT);
	return (0);
}