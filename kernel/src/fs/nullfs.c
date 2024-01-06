/**
 * @file drv/fs/tarfs.c
 * @author Пиминов Никита (nikita.piminoff@yandex.ru)
 * @brief Файловая система NullFS
 * @version 0.3.4
 * @date 2023-10-14
 * @copyright Copyright SayoriOS Team (c) 2022-2023
*/

#include <kernel.h>
#include <io/ports.h> 


size_t fs_nullfs_read(const char Disk,const char* Path, size_t Offset, size_t Size,void* Buffer){
	return 0;
}

size_t fs_nullfs_write(const char Disk,const char* Path,size_t Offset,size_t Size,void* Buffer){
	return 0;
}

FSM_FILE fs_nullfs_info(const char Disk,const char* Path){
	
}

FSM_DIR* fs_nullfs_dir(const char Disk,const char* Path){
	
}

int fs_nullfs_create(const char Disk,const char* Path,int Mode){
	return 0;
}

int fs_nullfs_delete(const char Disk,const char* Path,int Mode){
	return 0;
}

void fs_nullfs_label(const char Disk, char* Label){
	memcpy(Label,"NullFS",strlen("NullFS"));
}

int fs_nullfs_detect(const char Disk){
	return (0);
}