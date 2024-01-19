/**
 * @file drv/disk/initrd.c
 * @author Пиминов Никита (nikita.piminoff@yandex.ru)
 * @brief Файл виртуального диска, основаного на основе TarFS
 * @version 0.3.5
 * @date 2023-08-03
 * @copyright Copyright SayoriOS Team (c) 2022-2024
*/

#include <io/ports.h> 
#include <fs/fsm.h>
#include <fs/tarfs.h>
#include "drv/disk/dpm.h"
#include "mem/vmm.h"

#define INITRD_RW_SIZE (2*1024*1024) ///< Размер виртуального диска 2mb

int initrd_tarfs(uint32_t start,uint32_t end){
	qemu_log("[TarFS] Init...\n * Start: %x\n * End: %x\n * Size: %d",start,end,end-start);

	size_t initrd_size = end - start;
	if (start > end)
		return 0;

	void* initrd_data = (void*)start;

	qemu_warn("Initrd occupies %d pages", ALIGN(initrd_size, 4096) / 4096);

	TarFS_ROOT* l_initrd = fs_tarfs_init((uint32_t) initrd_data, initrd_size, 1);

	if (l_initrd->Ready == 0)
		return 0;

// 	qemu_log("dpm_metadata_read:%x ",dpm_metadata_read('R'));
// 
 	dpm_reg('R', "RamDisk", "TARFS", 2, initrd_size, 0, 0, 2, "TAR0-FSV1", initrd_data);
	dpm_metadata_write('R', (uint32_t) l_initrd);

	qemu_log("[INITRD] Create virtual read-write disk...");
	void* disk_t = kmalloc(INITRD_RW_SIZE+1);
	if (disk_t == NULL){
		qemu_log("[INITRD] Fatal create virtual disk");
		return 0;
	}
	qemu_log("[INITRD] Temp disk is (%d bytes) created to %x", INITRD_RW_SIZE, disk_t);

	
 	dpm_reg('T',"TempDisk","SMFS",2,INITRD_RW_SIZE,0,0,2,"TEMP-DISK",disk_t);
	//dpm_metadata_write('R',l_initrd);

	//fs_smfs_format('T');

// 	qemu_log("dpm_metadata_read:%x ",dpm_metadata_read('R'));
// 	qemu_log("l_initrd:%x ",l_initrd);
// 
// 	FSM_FILE file = fsm_info(fsm_getIDbyName("TARFS"),'R',"main.c");
////
// 	qemu_log("[FF Test] Name: %s",file.Name);
// 	qemu_log("[FF Test] Size: %d",file.Size);
// 	qemu_log("[FF Test] Type: %d",file.Type);
// 	qemu_log("[FF Test] Mode: %d",file.Mode);
// 
// 
// 	char* b_mainc = kmalloc(file.Size);
// 	size_t r_mainc = fsm_read(fsm_getIDbyName("TARFS"),'R',"main.c",10,5,b_mainc);
// 	qemu_log("[TEST] [TarFS] Content:\n%s\n",b_mainc);
// 
// 	qemu_log("TARFS:%d|%d",fsm_getIDbyName("TARFS"),fsm_getIDbyName("TarFS"));
	return 1;
}

