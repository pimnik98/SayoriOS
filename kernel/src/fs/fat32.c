/**
 * @file drv/fs/fat32.c
 * @author Павленко Андрей (pikachu.andrey@vk.com)
 * @brief Файловая система FAT32
 * @version 0.3.4
 * @date 2023-12-26
 * @copyright Copyright SayoriOS Team (c) 2022-2023
*/

#include <common.h>
#include <fs/fat32.h>
#include <io/ports.h>
#include <lib/string.h>

#include "drv/disk/dpm.h"
#include "mem/vmm.h"
#include "lib/utf_conversion.h"

size_t fs_fat32_read(char Disk, const char* Path, size_t Offset, size_t Size,void* Buffer){
	return 0;
}

size_t fs_fat32_write(char Disk, const char* Path,size_t Offset,size_t Size,void* Buffer){
	return 0;
}

FSM_FILE fs_fat32_info(char Disk, const char* Path) {
	return (FSM_FILE){};
}

FSM_DIR* fs_fat32_dir(char Disk,const char* Path){
    fat_description_t* desc = dpm_metadata_read(Disk);

    FSM_DIR *Dir = kcalloc(sizeof(FSM_DIR), 1);
    FSM_FILE *Files = kcalloc(sizeof(FSM_FILE), 1);

    // TODO: Actually provide an information about disk

    Dir->Ready = 1;
    Dir->Count = 0;
    Dir->CountFiles = 0;
    Dir->CountDir = 0;
    Dir->CountOther = 0;
    Dir->Files = Files;

    return Dir;
}

int fs_fat32_create(char Disk,const char* Path,int Mode){
	return 0;
}

int fs_fat32_delete(char Disk,const char* Path,int Mode){
	return 0;
}

void fs_fat32_label(char Disk, char* Label){
    fat_description_t* desc = dpm_metadata_read(Disk);

	memcpy(Label, desc->info.volume_label, 11);
}

vector_t* fs_fat32_get_clusters(char Disk, size_t cluster_number) {
    fat_description_t* desc = dpm_metadata_read(Disk);

    vector_t* container = vector_new();

    uint32_t cur_cluster = cluster_number;

    do {
        uint32_t old_cluster = cur_cluster;

        dpm_read(Disk, desc->fat_offset + (cur_cluster * 4), 4, &cur_cluster);

        vector_push_back(container, old_cluster);
    } while(!(cur_cluster == 0x0fffffff || cur_cluster == 0x0ffffff8));

    return container;
}

// Make sure buffer size is cluster-size aligned :)
void fs_fat32_read_clusters_to_memory(char Disk, size_t cluster_number, void* buffer) {
    fat_description_t* desc = dpm_metadata_read(Disk);

    vector_t* cluster_list = fs_fat32_get_clusters(Disk, cluster_number);

    for(int i = 0; i < cluster_list->size; i++) {
        size_t current_cluster = vector_get(cluster_list, i).element;

        size_t addr = ((desc->info.reserved_sectors + (desc->info.fat_size_in_sectors * 2)) \
 						+ ((current_cluster - 2) * desc->info.sectors_per_cluster)) * desc->info.bytes_per_sector;

        dpm_read(Disk, addr, desc->cluster_size, (void*)(((size_t)buffer) + (i * desc->cluster_size)));
    }

    vector_destroy(cluster_list);
}

size_t fs_fat32_get_cluster_count(char Disk, size_t cluster_number) {
    vector_t* clusters = fs_fat32_get_clusters(Disk, cluster_number);

    size_t cluster_count = clusters->size;

    vector_destroy(clusters);

    return cluster_count;
}

size_t fs_fat32_read_lfn(char* data, char* out) {
    size_t encoded_characters = 0;

    uint16_t* chunk = kcalloc(sizeof(uint16_t), 13);
    LFN_t lfn = {0};

    memset(out, 0, 256);

    while(true) {
        memcpy(&lfn, data, sizeof lfn);

        data += sizeof lfn;

        uint8_t lfn_num = lfn.attr_number & ~LFN_LAST_ENTRY;

        if(lfn.reserved != 0 || lfn_num > 20 || lfn.attribute != 0x0F) {
            // It's normal
            // qemu_err("Invalid LFN!");
            return 0;
        }

        memcpy(chunk, lfn.first_name_chunk, 10);
        memcpy(chunk + 5, lfn.second_name_chunk, 12);
        memcpy(chunk + 11, lfn.third_name_chunk, 4);

        uint16_t* prepared_chunk = kcalloc(sizeof(uint16_t), 13);

        for(int i = 0; i < 13; i++) {
            if(chunk[i] == 0xffff) {
                break;
            } else {
                prepared_chunk[i] = chunk[i];
                encoded_characters += 2;
            }
        }

//        char* x = new char[encoded_characters];
        char* x = kcalloc(1, encoded_characters);

        utf16_to_utf8((short*)prepared_chunk,
                      (int)encoded_characters / 2,
                      x);

//        qemu_note("[%d] PIECE: %.13s", encoded_characters, x);

        memmove(out + 13, out, strlen(x));
        memcpy(out, x, 13);

        kfree(prepared_chunk);
        kfree(x);

        memset(chunk, 0, 26);

        if(lfn_num == 1) // Is last?
            break;
    }

    kfree(chunk);

    return encoded_characters;
}

fat_file_info_t fs_fat32_get_file_info(char* data) {
    fat_file_info_t info = {0};

    size_t ecc = fs_fat32_read_lfn(data, info.filename);

    size_t byte_count = ecc / 2;

    size_t div = byte_count / 13, rem = byte_count % 13;

    if(rem > 0) div++;

    // Skip LFN and get into the file info section
    data += sizeof(LFN_t) * div;

    memcpy(&info.advanced_info, data, sizeof info.advanced_info);

    if(strlen(info.filename) == 0) {
        for(int i = 0; i < 11; i++) {
//            if(info.advanced_info.short_file_name[i] == ' ')
//                break;

            info.filename[i] = info.advanced_info.short_file_name[i];
        }

//        qemu_note("TODO: Read SFN and make it filename!");
//        while(1);
        info.is_lfn = false;
    } else {
        info.is_lfn = true;
    }

    info.starting_cluster = (info.advanced_info.first_cluster_high << 16) | info.advanced_info.first_cluster_low;
    info.size = info.advanced_info.file_size_in_bytes;

    return info;
}

void fs_fat32_scan_directory(char Disk, size_t directory_cluster) {
    fat_description_t* desc = dpm_metadata_read(Disk);

    size_t cluster_count = fs_fat32_get_cluster_count(Disk, directory_cluster);
    char* cluster_data = kcalloc(1, desc->cluster_size * cluster_count);

    fs_fat32_read_clusters_to_memory(Disk, directory_cluster, cluster_data);

    size_t offset = 0;

    // TODO: Actually scan directory
    while(1) {
        fat_file_info_t file = fs_fat32_get_file_info(cluster_data + offset);

        if(file.advanced_info.short_file_name[0] == 0)
            break;

        size_t len = strlen(file.filename);
        size_t skip_lfns_count = (len / 13);
        size_t skip_lfns_remiander = (len % 13);

        if(skip_lfns_remiander > 0)
            skip_lfns_count++;

        if(file.is_lfn)
            offset += sizeof(LFN_t) * skip_lfns_count;

        offset += sizeof(fat_object_info_t);

        qemu_note("%s %d", file.filename, file.size);
    }

//    while(1);

    kfree(cluster_data);
}

int fs_fat32_detect(char Disk) {
// If we initialize fat32 again, bug will appear (could not read root directory)
//    if(dpm_metadata_read(Disk)) {
//        qemu_err("WHAT?");
//        while(1);
//    }

    fat_description_t* fat_system = kcalloc(1, sizeof(fat_description_t));

	dpm_read(Disk, 0, sizeof(fat_info_t), &fat_system->info);

    qemu_warn("Trying FAT32...");

    bool is_fat_bootcode = (unsigned char)fat_system->info.bootcode[0] == 0xEB
                           && (unsigned char)fat_system->info.bootcode[1] == 0x58
                           && (unsigned char)fat_system->info.bootcode[2] == 0x90;

    bool is_fat_fsname = memcmp(fat_system->info.fs_type, "FAT32", 0) == 0;

    if(is_fat_bootcode && is_fat_fsname) {
        qemu_note("FAT filesystem found!");
        qemu_note("Disk label: %.11s", fat_system->info.volume_label);

        // Calculate needed values
        fat_system->cluster_size = fat_system->info.bytes_per_sector * fat_system->info.sectors_per_cluster;
        fat_system->fat_offset = fat_system->info.reserved_sectors * fat_system->info.bytes_per_sector;
        fat_system->fat_size = fat_system->info.fat_size_in_sectors * fat_system->info.bytes_per_sector;
        fat_system->reserved_fat_offset = (fat_system->info.reserved_sectors + fat_system->info.fat_size_in_sectors) * fat_system->info.bytes_per_sector;
        fat_system->root_directory_offset = ((fat_system->info.reserved_sectors + (fat_system->info.fat_size_in_sectors * 2)) + ((fat_system->info.root_directory_offset_in_clusters - 2) * fat_system->info.sectors_per_cluster)) * fat_system->info.bytes_per_sector;

        qemu_note("Cluster size: %d", fat_system->cluster_size);
        qemu_note("FAT offset: %d", fat_system->fat_offset);
        qemu_note("FAT size: %d", fat_system->fat_size);
        qemu_note("Reserved FAT offset: %d", fat_system->reserved_fat_offset);
        qemu_note("Root directory offset: %d", fat_system->root_directory_offset);

        // Assign FAT32 filesystem in-place
        dpm_metadata_write(Disk, (uint32_t) fat_system);

        vector_t* root_directory = fs_fat32_get_clusters(Disk, 2);

        qemu_note("[%d] Root occupies folllwing clusters:", root_directory->size);
        for(int i = 0; i < root_directory->size; i++) {
            qemu_note("Cluster: %d", vector_get(root_directory, i).element);
        }

        vector_destroy(root_directory);

        qemu_warn("SCANNING ROOT DIRECTORY");
        fs_fat32_scan_directory(Disk, 2);
        qemu_warn("END SCANNING ROOT DIRECTORY");

        return 1;
    }

	return 0;
}