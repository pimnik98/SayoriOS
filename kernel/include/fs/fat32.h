/**
 * @file drv/fs/fat32.c
 * @author Павленко Андрей (pikachu_andrey@vk.com)
 * @brief Файловая система FAT32
 * @version 0.3.4
 * @date 2023-11-04
 * @copyright Copyright SayoriOS Team (c) 2022-2023
*/
#pragma once

#include <common.h>
#include <io/ports.h>
#include <fs/fsm.h>
#include "../../src/lib/libvector/include/vector.h"

typedef struct {
    char bootcode[3];
    char OEM[8];
    uint16_t  bytes_per_sector;
    uint8_t   sectors_per_cluster;
    uint16_t  reserved_sectors;
    uint8_t   copies;
    uint16_t  root_entries;
    uint16_t  small_sectors_number;
    uint8_t   descriptor;
    uint16_t  sectors_per_fat;
    uint16_t  sectors_per_track;
    uint16_t  heads;
    uint32_t  hidden_sectors;
    uint32_t  sectors_in_partition;
    uint32_t  fat_size_in_sectors;
    uint16_t flags;
    uint16_t version_num;
    uint32_t  root_directory_offset_in_clusters;
    uint16_t fsinfo_sector;
    uint16_t  _;
    char reserved1[12];
    uint8_t disk_number;
    uint8_t  flags1;
    uint8_t   extended_boot_signature;
    uint32_t  volume_serial_number;
    char volume_label[11];
    char fs_type[8];
} __attribute__((packed)) fat_info_t;

typedef struct {
    char short_file_name[11];  // 8.3
    uint8_t attributes;
    uint8_t register_sign;

    uint8_t create_millis;
    uint16_t create_time;
    uint16_t create_date;
    uint16_t last_access_date;

    uint16_t first_cluster_high;
    uint16_t last_modif_time;
    uint16_t last_modif_date;
    uint16_t first_cluster_low;
    uint32_t file_size_in_bytes;
} __attribute__((packed)) fat_object_info_t;

typedef struct {
    uint8_t attr_number;
    uint16_t first_name_chunk[5];
    uint8_t attribute;
    uint8_t reserved;
    uint8_t checksum;
    uint16_t second_name_chunk[6];
    uint16_t reserved2;
    uint16_t third_name_chunk[2];
} __attribute__((packed)) LFN_t;

typedef struct {
    fat_info_t info;

    uint32_t cluster_size;
    uint32_t fat_offset;
    uint32_t fat_size;
    uint32_t reserved_fat_offset;
    uint32_t root_directory_offset;
} fat_description_t;  // This structure never used in data parsing and represents all needed data for FAT32 driver.

#define LFN_LAST_ENTRY 0x40

struct fat32_lfn_result {
    char* filename;
    size_t encoded_characters_count;
};

typedef struct {
    char filename[256];
    size_t size;

    size_t starting_cluster;

    fat_object_info_t advanced_info;

    bool is_lfn;
} fat_file_info_t;

size_t fs_fat32_read(char Disk,const char* Path, size_t Offset, size_t Size,void* Buffer);
size_t fs_fat32_write(char Disk,const char* Path,size_t Offset,size_t Size,void* Buffer);
FSM_FILE fs_fat32_info(char Disk,const char* Path);
FSM_DIR* fs_fat32_dir(char Disk,const char* Path);
int fs_fat32_create(char Disk,const char* Path,int Mode);
int fs_fat32_delete(char Disk,const char* Path,int Mode);
void fs_fat32_label(char Disk, char* Label);
int fs_fat32_detect(char Disk);
vector_t* fs_fat32_get_clusters(char Disk, size_t cluster_number);