// SAFF (SAyori Font File) by NDRAEY 2022

#ifndef SAFF_H
#define SAFF_H

// Exyl - Ping 2!

static const char valid_magic[5] = {'S', 'A', 'F', 'F', '1'};

typedef struct SAFF_FMT {
    char magic[5]; // SAFF1
    char parameter_byte; // 0x01
    char font_count_byte; // 1
    char font_count; // 0-255
    char kerning_byte; // 2
    char kerning; // 0-255 px
    unsigned short width;
    unsigned short height;
    char reserved[16];
} SAFF_FMT_t;

typedef struct SAFF_LOAD {
    SAFF_FMT_t* meta;
    char* raw;
    char* data;
} SAFF_LOAD_t;

#endif