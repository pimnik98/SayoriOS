#pragma once

#include <kernel.h>

typedef struct DANHeader {
    unsigned char magic[3];
    uint32_t frame_count;
    uint16_t width;
    uint16_t height;
    uint8_t alpha;
} __attribute__((packed)) DANHeader_t;

typedef struct DANDescriptor {
    DANHeader_t header;
    uint32_t framesize;
    char* data;

    bool stream;
    FILE* fd;
} DANDescriptor_t;

DANDescriptor_t* alloc_dan(const char* filename, bool stream);
void read_frame_dan(DANDescriptor_t* dan, size_t index, char* out);
void free_dan(DANDescriptor_t* dan);