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
size_t file_descriptor_allocate(const char *filename, size_t mode, int *out);
size_t file_descriptor_read(int descriptor_number, size_t count, void* buffer);
size_t file_descriptor_write(int descriptor_number, size_t count, const void* buffer);
size_t file_descriptor_close(int descriptor_number);
size_t file_descriptor_seek(int descriptor_number, ssize_t value, size_t whence);
size_t file_descriptor_tell(int descriptor_number, int* out);
