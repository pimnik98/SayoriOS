#pragma once

#include <kernel.h>
#include <common.h>
/*
 * SFF1 File Specification
 * 
 * SFF Common (Global) header:
 * |- Identifier: "SFF"
 * |- Version: 1 (uint8_t)
 * 
 * SFF1 Header:
 * |- Glyph Width (uint8_t)
 * |- Glyph Height (uint8_t)
 * |- Duke identifier: "DUKE"
 */

#define ARRAY_CHARACTER_COUNT 148
#define IS_IN_SFF1(chr) (is_in_sff1_alphabet_array(chr, alphabet_sff1, ARRAY_CHARACTER_COUNT) != 0xffff)
#define UTF8_GLOBAL_SHIFT 0xcc80
#define UTF8_GLOBAL_SHIFT_2 0xcd41

typedef struct {
    uint8_t identifier[3];
    uint8_t version;
} __attribute__((packed)) SFF_Global_Header_t;

typedef struct {
    uint8_t glyph_width;
    uint8_t glyph_height;

    uint8_t DUKE_identifier[4];
} __attribute__((packed)) SFF1_Header_t;

typedef struct {
    SFF_Global_Header_t* global;
    
    // Data for SFF1
    SFF1_Header_t* sff1;
    uint8_t* image_data;
    struct DukeImageMeta* image_metadata;

    FILE* file_descriptor;
} SFF_Descriptor_t;

SFF_Descriptor_t* load_sff_font(const char* path);
uint16_t is_in_sff1_alphabet_array(uint16_t character, uint16_t* array, uint16_t length);
void _draw_sff1_char_screen(SFF_Descriptor_t* descriptor, uint16_t character, 
                            size_t x, size_t y, uint32_t color);
void _draw_sff1_string_screen(SFF_Descriptor_t* descriptor, uint8_t* string,
                              size_t x, size_t y, uint32_t color);
void scale_sff1_font(SFF_Descriptor_t* descriptor, size_t w, size_t h);
void destroy_sff_font(SFF_Descriptor_t* descriptor);