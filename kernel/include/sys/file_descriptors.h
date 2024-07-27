//
// Created by maractus on 03.01.24.
//

#pragma once

#include "lib/stdio.h"

struct fd_info {
    int fd;
    FILE* file;
};

void file_descriptors_init();
void file_descriptor_allocate(const char *filename, size_t mode, int *out);
void file_descriptor_read(int descriptor_number, size_t count, void* buffer);
void file_descriptor_write(int descriptor_number, size_t count, const void* buffer);
void file_descriptor_close(int descriptor_number);
void file_descriptor_seek(int descriptor_number, ssize_t value, size_t whence);
void file_descriptor_tell(int descriptor_number, int* out);
