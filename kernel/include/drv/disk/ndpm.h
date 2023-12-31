//
// Created by ndraey on 23.12.23.
//

#pragma once
#include "common.h"

typedef struct ndpm_drive ndpm_drive_t;

typedef void (*ndpm_read_fn_t)(const ndpm_drive_t*, size_t location, int size, void* buffer);
typedef void (*ndpm_write_fn_t)(const ndpm_drive_t*, size_t location, int size, void* buffer);

typedef struct ndpm_drive {
    void* drive_specific_data;
    ndpm_read_fn_t read_fn;
    ndpm_write_fn_t write_fn;
} ndpm_drive_t;

void ndpm_init();
void ndpm_add_drive(void* specific_data, ndpm_read_fn_t reader, ndpm_write_fn_t writer);
ndpm_drive_t* ndpm_get_drive(int index);

SAYORI_INLINE void ndpm_read(const ndpm_drive_t* drive, size_t location, int size, void* buffer) {
    drive->read_fn(drive, location, size, buffer);
}

SAYORI_INLINE void ndpm_write(const ndpm_drive_t* drive, size_t location, int size, void* buffer) {
    drive->write_fn(drive, location, size, buffer);
}