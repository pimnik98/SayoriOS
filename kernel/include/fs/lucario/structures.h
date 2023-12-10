#pragma once

#include <common.h>

#define LUCARIOFS_MAGIC "LUCARIO"

typedef enum {
    E_NONE = 0x00,
    E_FILE = 0xF1,
    E_FOLDER = 0xF0
} LucarioFileEntryType_t;

typedef struct {
    char magic[7];  // LUCARIO
    char version_major;
    char version_minor;
    char version_patch;
} __attribute__((packed)) LucarioHeader_t;

typedef struct {
    uint8_t type;
    char name[256];
    uint32_t folder_target_id;
    uint32_t folder_id;
    uint32_t sector_list_lba;
    uint32_t sector_list_size;
    uint32_t file_size;
} __attribute__((packed)) LucarioFileEntry_t;

typedef struct {
    bool ok;
    uint8_t ata_drive;
    size_t disk_capacity;

    LucarioHeader_t header;
    size_t max_entries;
} LucarioDescriptor_t;
