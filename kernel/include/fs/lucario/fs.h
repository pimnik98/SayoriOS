#pragma once

#include <fs/lucario/structures.h>

#define LUCARIO_DIVISOR (8192)

LucarioDescriptor_t* lucario_fs_build_descriptor();
bool lucario_fs_init(LucarioDescriptor_t* descr, uint8_t ata_drive);
void lucario_fs_destroy_descriptor(LucarioDescriptor_t* descr);
void lucario_fs_read_file_entry(LucarioDescriptor_t* descr, size_t index, LucarioFileEntry_t* out);
void lucario_fs_get_file_entry(LucarioDescriptor_t* descr, char name[], size_t folder_id, LucarioFileEntry_t* out);
bool lucario_fs_file_exists(LucarioDescriptor_t* descr, char name[], size_t folder_id);
size_t lucario_fs_file_size(LucarioDescriptor_t* descr, char name[], size_t folder_id);

static inline size_t lucario_fs_sector_list_pos_byte(LucarioDescriptor_t* descr, LucarioFileEntry_t* entry) {
    return entry->sector_list_lba * 512;
}

bool lucario_fs_read_file(LucarioDescriptor_t* descr, char name[], size_t folder_id, size_t offset, size_t length, char* out);