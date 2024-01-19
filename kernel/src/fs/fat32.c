/**
 * @file drv/fs/fat32.c
 * @author Павленко Андрей (pikachu.andrey@vk.com)
 * @brief Файловая система FAT32
 * @version 0.3.5
 * @date 2023-12-26
 * @copyright Copyright SayoriOS Team (c) 2022-2024
*/

#include <common.h>
#include <fs/fat32.h>
#include <io/ports.h>
#include <lib/string.h>

#include "drv/disk/dpm.h"
#include "mem/vmm.h"
#include "lib/utf_conversion.h"
#include "lib/math.h"
#include "fmt/tga.h"
#include "io/rgb_image.h"
#include "lib/libstring/string.h"
#include "lib/php/pathinfo.h"

size_t fs_fat32_read(char Disk, const char* Path, size_t Offset, size_t Size,void* Buffer){
    size_t clust = fs_fat32_evaluate(Disk, Path, true);

    if(clust == 0) {
        return 0;
    }

    char* filename = pathinfo(Path, PATHINFO_BASENAME);

//    fat_file_info_t info = fs_fat32_get_object_info(Disk, filename, clust);

    fs_fat32_read_file_from_dir(Disk, clust, Offset, Size, filename, Buffer);

    kfree(filename);

	return Size;
}

size_t fs_fat32_write(char Disk, const char* Path,size_t Offset,size_t Size,void* Buffer){
	return 0;
}

FSM_FILE fs_fat32_info(char Disk, const char* Path) {
    FSM_FILE file = {0};

    // Get folder where file is contained
    size_t clust = fs_fat32_evaluate(Disk, Path, true);

    if(clust == 0) {
        qemu_err("File does not exist");
        return file;
    }

    char* filename = pathinfo(Path, PATHINFO_BASENAME);

    qemu_warn("Path: %s; Filename: %s;", Path, filename);

    fat_file_info_t info = fs_fat32_get_object_info(Disk, filename, clust);
    qemu_log("OK?");

    if(info.filename[0] == 0) {
        return file;
    }

    memset(file.Name, 0, 1024);
    memcpy(file.Name, info.filename, 256);

    file.Size = info.size;
    // PIMNIK98 MAKE MACROS FOR FSM TYPES PLEASE
    //    file.Type = WHAT?????

    file.Ready = 1;

    qemu_ok("%s %d", file.Name, file.Size);

    kfree(filename);

    return file;
}

FSM_DIR* fs_fat32_dir(char Disk, const char* Path) {
    fat_description_t* desc = dpm_metadata_read(Disk);

    qemu_note("Given path is: %s", Path);
    size_t clust = fs_fat32_evaluate(Disk, Path, true);

    qemu_warn("Got cluster: %d", clust);

    FSM_DIR *Dir = kcalloc(sizeof(FSM_DIR), 1);

    if(clust == 0) {
        Dir->Ready = 0;
        Dir->Count = 0;
        Dir->CountFiles = 0;
        Dir->CountDir = 0;
        Dir->CountOther = 0;
        Dir->Files = 0;

        return Dir;
    }

    FSM_FILE *Files = kcalloc(sizeof(FSM_FILE), 1);

    size_t cluster_count = fs_fat32_get_cluster_count(Disk, clust);
    char* cluster_data = kcalloc(desc->cluster_size, cluster_count);

    fs_fat32_read_clusters_to_memory(Disk, clust, cluster_data);

    size_t offset = 0;
    size_t file_count = 0;
    size_t directory_count = 0;

    while(1) {
        fat_file_info_t file = fs_fat32_read_file_info(cluster_data + offset);

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

        Files = krealloc(Files, sizeof(FSM_FILE) * (file_count + directory_count + 1));

        Files[directory_count + file_count].LastTime.year = 1980 + ((file.advanced_info.last_modif_date >> 9) & 0b1111111);
        Files[directory_count + file_count].LastTime.month = (file.advanced_info.last_modif_date >> 5) & 0b1111;
        Files[directory_count + file_count].LastTime.day = (file.advanced_info.last_modif_date >> 0) & 0b11111;

        Files[directory_count + file_count].LastTime.hour = ((file.advanced_info.last_modif_time >> 11) & 0b11111);
        Files[directory_count + file_count].LastTime.minute = ((file.advanced_info.last_modif_time >> 5) & 0b111111);
        Files[directory_count + file_count].LastTime.second = ((file.advanced_info.last_modif_time >> 0) & 0b11111);

        memset(Files[directory_count + file_count].Name, 0, 1024);
        memcpy(Files[directory_count + file_count].Name, file.filename, strlen(file.filename));
        Files[directory_count + file_count].Size = file.size;
        Files[directory_count + file_count].Ready = 1;

        qemu_note("File: %s, Size: %d", Files[directory_count + file_count].Name, Files[directory_count + file_count].Size);

        if(file.advanced_info.attributes & ATTR_DIRECTORY) {
            Files[directory_count + file_count].Type = 5;  // What an undocumented magic?

            directory_count++;
        } else {
            Files[directory_count + file_count].Type = 0;  // What an undocumented magic?

            file_count++;
        }
    }

    kfree(cluster_data);

    Dir->Ready = 1;
    Dir->Count = file_count + directory_count;
    Dir->CountFiles = file_count;
    Dir->CountDir = directory_count;
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

        cur_cluster = desc->fat_table[cur_cluster];
//        dpm_read(Disk, desc->fat_offset + (cur_cluster * 4), 4, &cur_cluster);

        vector_push_back(container, old_cluster);
    } while(!(cur_cluster == 0x0fffffff || cur_cluster == 0x0ffffff8));

    return container;
}

void fs_fat32_read_entire_fat(char Disk) {
    fat_description_t* desc = dpm_metadata_read(Disk);

    if(desc->fat_table) {
        qemu_err("Reading FAT second time is not allowed");
        while(1);
    }

    desc->fat_table = kcalloc(1, desc->fat_size);

    dpm_read(Disk, desc->fat_offset, desc->fat_size, desc->fat_table);
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

void fs_fat32_read_clusters_to_memory_precise(char Disk, size_t cluster_number, void *buffer, size_t byte_offset,
                                              size_t len) {
    fat_description_t* desc = dpm_metadata_read(Disk);

    qemu_log("Reading cluster chain...");
    vector_t* cluster_list = fs_fat32_get_clusters(Disk, cluster_number);

    qemu_log("Byte offset: %d; Size of read: %d; Cluster size: %d", byte_offset, len, desc->cluster_size);

    size_t starting_cluster = byte_offset / desc->cluster_size;
    size_t read_clutser_count = len / desc->cluster_size;

    if(len % desc->cluster_size > 0) {
        read_clutser_count++;
    }

    qemu_log("Calculated: Starting cluster: %d; Cluster count: %d", starting_cluster, read_clutser_count);

    qemu_log("Reading file data...");
//    for(int i = 0; i < cluster_list->size; i++) {
    for(size_t i = starting_cluster; i < starting_cluster + read_clutser_count; i++) {
        size_t buffer_index = i - starting_cluster;

        size_t current_cluster = vector_get(cluster_list, i).element;

        size_t addr = ((desc->info.reserved_sectors + (desc->info.fat_size_in_sectors * 2)) \
 						+ ((current_cluster - 2) * desc->info.sectors_per_cluster)) * desc->info.bytes_per_sector;

        dpm_read(
                Disk,
                addr + byte_offset,
                MIN(desc->cluster_size, len),
                (void*)(((size_t)buffer) + (buffer_index * desc->cluster_size))
        );

        len -= desc->cluster_size;
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
            // It's normal (it may indicate that LFN entries are coming to an end.
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

        char* x = kcalloc(1, encoded_characters);

        utf16_to_utf8((short*)prepared_chunk,
                      (int)encoded_characters / 2,
                      x);

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

fat_file_info_t fs_fat32_read_file_info(char* data) {
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

        info.is_lfn = false;
    } else {
        info.is_lfn = true;
    }

    info.starting_cluster = (info.advanced_info.first_cluster_high << 16) | info.advanced_info.first_cluster_low;
    info.size = info.advanced_info.file_size_in_bytes;

    return info;
}

fat_file_info_t fs_fat32_get_object_info(char Disk, const char* filename, size_t directory_cluster) {
    fat_description_t* desc = dpm_metadata_read(Disk);

    size_t cluster_count = fs_fat32_get_cluster_count(Disk, directory_cluster);

    char* cluster_data = kcalloc(desc->cluster_size, cluster_count);

    fs_fat32_read_clusters_to_memory(Disk, directory_cluster, cluster_data);

    size_t offset = 0;

    while(1) {
        fat_file_info_t info = fs_fat32_read_file_info(cluster_data + offset);

        if(info.advanced_info.short_file_name[0] == 0)
            break;

        size_t len = strlen(info.filename);
        size_t skip_lfns_count = (len / 13);
        size_t skip_lfns_remiander = (len % 13);

        if(skip_lfns_remiander > 0)
            skip_lfns_count++;

        if(info.is_lfn)
            offset += sizeof(LFN_t) * skip_lfns_count;

        offset += sizeof(fat_object_info_t);

        if(strcmp(info.filename, filename) == 0) {
            kfree(cluster_data);

            return info;
        }
    }

    kfree(cluster_data);

    return (fat_file_info_t){0};
}

void fs_fat32_read_file_from_dir(char Disk, size_t directory_cluster, size_t byte_offset, size_t length, char *filename,
                                 char *out) {
    fat_description_t* desc = dpm_metadata_read(Disk);

    size_t cluster_count = fs_fat32_get_cluster_count(Disk, directory_cluster);
    char* cluster_data = kcalloc(desc->cluster_size, cluster_count);

    fs_fat32_read_clusters_to_memory(Disk, directory_cluster, cluster_data);

    size_t offset = 0;

    while(1) {
        fat_file_info_t file = fs_fat32_read_file_info(cluster_data + offset);

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

        if(memcmp(file.filename, filename, len) == 0) {
            qemu_ok("File found!");

//            fs_fat32_read_clusters_to_memory_precise(Disk, file.starting_cluster, out, byte_offset, file.size);
            fs_fat32_read_clusters_to_memory_precise(Disk, file.starting_cluster, out, byte_offset, length);
            break;
        }
    }

    kfree(cluster_data);
}

void fs_fat32_scan_directory(char Disk, size_t directory_cluster) {
    fat_description_t* desc = dpm_metadata_read(Disk);

    size_t cluster_count = fs_fat32_get_cluster_count(Disk, directory_cluster);
    char* cluster_data = kcalloc(desc->cluster_size, cluster_count);

    fs_fat32_read_clusters_to_memory(Disk, directory_cluster, cluster_data);

    size_t offset = 0;

    // TODO: Actually scan directory
    while(1) {
        fat_file_info_t file = fs_fat32_read_file_info(cluster_data + offset);

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

// Return cluster number
size_t fs_fat32_evaluate(char Disk, const char* path, bool error_on_file) {
    size_t current_cluster = 2;  // 2 is a root directory
    size_t old_cluster = 2;

    if(strlen(path) == 0) {
        return current_cluster;
    }

    string_t* strpath = string_from_charptr(path);
    vector_t* pieces = string_split(strpath, "/");

    fat_file_info_t temp_info = {0};

    for(int i = 0; i < pieces->size; i++) {
        char* value = ADDR2STRING(pieces->data[i])->data;
        qemu_note("%s", value);

        if(strlen(value) == 0)
            continue;

        if(strcmp(value, ".") == 0)
            continue;

        if(strcmp(value, "..") == 0) {
            current_cluster = old_cluster;
            continue;
        }

        temp_info = fs_fat32_get_object_info(Disk, value, current_cluster);

        if(temp_info.filename[0] == 0) {
            string_split_free(pieces);
            string_destroy(strpath);

            return 0;
        }

        if(~temp_info.advanced_info.attributes & ATTR_DIRECTORY) {
            if(error_on_file) {
                qemu_err("That's not a directory, can't go on...");

                string_split_free(pieces);
                string_destroy(strpath);

                return current_cluster;
            } else {
                string_split_free(pieces);
                string_destroy(strpath);

                return temp_info.starting_cluster;
            }
        }

        old_cluster = current_cluster;
        current_cluster = temp_info.starting_cluster;
    }

    string_split_free(pieces);
    string_destroy(strpath);

    return current_cluster;
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

        qemu_log("Reading FAT32 FAT table to memory, it may take a while...");
        fs_fat32_read_entire_fat(Disk);

        vector_t* root_directory = fs_fat32_get_clusters(Disk, 2);

        qemu_note("[%d] Root occupies folllwing clusters:", root_directory->size);
        for(int i = 0; i < root_directory->size; i++) {
            qemu_note("Cluster: %d", vector_get(root_directory, i).element);
        }

        vector_destroy(root_directory);

        qemu_warn("SCANNING ROOT DIRECTORY");
        fs_fat32_scan_directory(Disk, 2);
        qemu_warn("END SCANNING ROOT DIRECTORY");

//        FSM_FILE inf = fs_fat32_info('C', "fight.tga");
//
//        char* temp = kcalloc(1, inf.Size);
//        char* pix = kcalloc(1, 6 * MB);
//        char* pix2 = kcalloc(1, 4 * MB);
//
//        fs_fat32_read(Disk, "fight.tga", 0, inf.Size, temp);
//
//        tga_extract_pixels_from_data(temp, pix);
//
//        size_t origw = 800, origh = 1245;
//        size_t targw = 400, targh = 600;
//
//        scale_rgb_image(pix, origw, origh, targw, targh, 1, pix2);
//
//        draw_rgb_image(pix2, targw, targh, 32, 0, 0);
//
//        kfree(temp);
//        kfree(pix);
//        kfree(pix2);
//
//        while(1);

        return 1;
    }

	return 0;
}
