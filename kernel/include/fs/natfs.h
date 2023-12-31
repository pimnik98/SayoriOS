#pragma once

#include <common.h>

int fs_natfs_detect(const char Disk);
void fs_natfs_label(const char Disk, char* Label);
int fs_natfs_delete(const char Disk,const char* Path,int Mode);
int fs_natfs_create(const char Disk,const char* Path,int Mode);
FSM_DIR* fs_natfs_dir(const char Disk,const char* Path);
FSM_FILE fs_natfs_info(const char Disk,const char* Path);
size_t fs_natfs_write(const char Disk,const char* Path,size_t Offset,size_t Size,void* Buffer);
size_t fs_natfs_read(const char Disk,const char* Path, size_t Offset, size_t Size,void* Buffer);
int fs_natfs_init();