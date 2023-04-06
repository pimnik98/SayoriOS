#include <kernel.h>
#include <common.h>

#include "gui/sayori_font_file.h"

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

uint16_t alphabet_sff1[ARRAY_CHARACTER_COUNT] = {
'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
'w', 'x', 'y', 'z', 1040, 1041, 1042, 1043,
1044, 1045, 1025, 1046, 1047, 1048, 1049, 1050,
1051, 1052, 1053, 1054, 1055, 1056, 1057, 1058,
1059, 1060, 1061, 1062, 1063, 1064, 1065, 1066,
1067, 1068, 1069, 1070, 1071, 1072, 1073, 1074,
1075, 1076, 1077, 1105, 1078, 1079, 1080, 1081,
1082, 1083, 1084, 1085, 1086, 1087, 1088, 1089,
1090, 1091, 1092, 1093, 1094, 1095, 1096, 1097,
1098, 1099, 1100, 1101, 1102, 1103, '.', ',',
'!', 8470, ';', '%', ':', '?', '*', '(',
')', '_', '+', '-', '=', '@', '#', '$',
'^', '&', '[', ']', '{', '}', '<', '>',
'|', '\\', '/', '~'
};

#define IN_RANGE(x, min, max) (x >= min && x <= max)

SFF_Descriptor_t* load_sff_font(const char* path) {
    FILE* file = fopen(path, "rb");
    if(!file) {
        qemu_log("File %s not found to load font.", path
        );
        return 0;
    }

    SFF_Descriptor_t* desc = kcalloc(1, sizeof(SFF_Descriptor_t));

    SFF_Global_Header_t* sff = kcalloc(1, sizeof(SFF_Global_Header_t));
    fread_c(file, 1, sizeof(SFF_Global_Header_t), sff);

    qemu_log("SFF version is: %d", sff->version);

    if(sff->version == 1) {
        SFF1_Header_t* sff1_hdr = kcalloc(1, sizeof(SFF1_Header_t));
        fread_c(file, 1, sizeof(SFF1_Header_t), sff1_hdr);

        desc->sff1 = sff1_hdr;

        if(memcmp(sff1_hdr->DUKE_identifier, DUKE_MAGIC, 4) == 0) {
            fseek(file, ftell(file) - 4, SEEK_SET);

            desc->image_metadata = kcalloc(1, sizeof(struct DukeImageMeta));
            fread_c(file, 1, sizeof(struct DukeImageMeta), desc->image_metadata);
            
            desc->image_data = kcalloc(1, desc->image_metadata->data_length);
            fread_c(file, desc->image_metadata->data_length, 1, desc->image_data);
        }else{
            qemu_log("SFF1: Not a valid font, but header is OK");
        }
    }

    qemu_log("Glyph Width: %d", desc->sff1->glyph_width);
    qemu_log("Glyph Height: %d", desc->sff1->glyph_height);
    qemu_log("Alpha: %d", desc->image_metadata->alpha);

    desc->global = sff;
    desc->file_descriptor = file;

    return desc;
}

uint16_t is_in_sff1_alphabet_array(uint16_t character, uint16_t* array, uint16_t length) {
    for (uint16_t i = 0; i < length; i++) {
        if(array[i] == character) {
            return i;
        }
    }

    return 0xffff;
}

/*
void _draw_sff1_char_to_buffer(SFF_Descriptor_t* descriptor, uint8_t* write_to,
                               uint16_t character, 
                               size_t x, size_t y, size_t width, uint32_t color) {
    if(!IS_IN_SFF1(character)) return;

    uint16_t charidx = is_in_sff1_alphabet_array(character, alphabet_sff1, ARRAY_CHARACTER_COUNT);
    uint16_t startpos = charidx * descriptor->sff1->glyph_width;
    
    if(!descriptor->image_metadata->alpha) {
        for(size_t i = 0; i < descriptor->sff1->glyph_height; i++) {  // i = y
            for(size_t j = 0; j < descriptor->sff1->glyph_width; j++) {  // j = x
                size_t pixpos_in_font = pixidx(descriptor->image_metadata->width*3, j*3, i);
                size_t pixpos_on_buffer = pixidx(width*3, (x+j)*3, y+i);

                uint8_t r = descriptor->image_data[pixpos_in_font];
                uint8_t g = descriptor->image_data[pixpos_in_font + 1];
                uint8_t b = descriptor->image_data[pixpos_in_font + 2];

                if(
                    IN_RANGE(r, 0xa0, 0xff)
                    && IN_RANGE(g, 0xa0, 0xff)
                    && IN_RANGE(b, 0xa0, 0xff)
                ) continue;

                //RGB_TO_UINT32(r, g, b);

                write_to[pixpos_on_buffer] = (color >> 16) & 255;
                write_to[pixpos_on_buffer + 1] = (color >> 8) & 255;
                write_to[pixpos_on_buffer + 2] = color & 255;
                // set_pixel(x+j, y+i, RGB_TO_UINT32(r, g, b));
            }
        }
    }else{
        // TODO: Alpha images support
    }
}
*/

void _draw_sff1_char_screen(SFF_Descriptor_t* descriptor, uint16_t character, 
                            size_t x, size_t y, uint32_t color) {
    if(!IS_IN_SFF1(character)) {
        qemu_log("%d not in array!", character);
        return;
    }

    qemu_log("%d in array!", character);

    uint16_t charidx = is_in_sff1_alphabet_array(character, alphabet_sff1, ARRAY_CHARACTER_COUNT);
    uint8_t mod = descriptor->image_metadata->alpha?4:3;
    uint16_t startpos = charidx * (descriptor->sff1->glyph_width * mod);
    
    if(!descriptor->image_metadata->alpha) {
        for(size_t i = 0; i < descriptor->sff1->glyph_height; i++) {  // i = y
            for(size_t j = 0; j < descriptor->sff1->glyph_width; j++) {  // j = x
                size_t pixpos_in_font = PIXIDX(descriptor->image_metadata->width*3, j*3, i) + startpos;

                uint8_t r = descriptor->image_data[pixpos_in_font];
                uint8_t g = descriptor->image_data[pixpos_in_font + 1];
                uint8_t b = descriptor->image_data[pixpos_in_font + 2];

                if(r == 0xff && g == 0xff && b == 0xff) continue;

                set_pixel(x+j, y+i, color);
            }
        }
    }else{
        // TODO: Alpha images support
    }
}

void _draw_sff1_string_screen(SFF_Descriptor_t* descriptor, uint8_t* string,
                              size_t x, size_t y, uint32_t color) {

    size_t rx = x, ry = y;
    for (size_t i = 0, len = strlen(string); i < len; i++) {
        if(string[i] == '\n') {
            i++;
            ry += descriptor->sff1->glyph_height;
            continue;
        }
        uint16_t utf8_char = ((string[i] << 8) | string[i + 1]) - UTF8_GLOBAL_SHIFT;
        utf8_char -= utf8_char >= 0x441 ? (UTF8_GLOBAL_SHIFT_2-UTF8_GLOBAL_SHIFT - 1) : 0;
        _draw_sff1_char_screen(descriptor, isUTF(string[i])?utf8_char:string[i], rx, ry, color);
        if(isUTF(string[i])){
            i++;
        }

        rx += descriptor->sff1->glyph_width;
    }
}

void scale_sff1_font(SFF_Descriptor_t* descriptor, size_t w, size_t h) {
    // TODO: Scale font using Duke's functional

    char* new = kmalloc(duke_calculate_bufsize(w*ARRAY_CHARACTER_COUNT, h, descriptor->image_metadata->alpha));

    duke_scale(descriptor->image_data,
               descriptor->image_metadata->width, descriptor->image_metadata->height,
               w*ARRAY_CHARACTER_COUNT, h, descriptor->image_metadata->alpha, new);

    kfree(descriptor->image_data);
    descriptor->image_data = new;

    descriptor->sff1->glyph_width = w;
    descriptor->sff1->glyph_height = h;

    descriptor->image_metadata->width = w*ARRAY_CHARACTER_COUNT;
    descriptor->image_metadata->height = h;
}

void destroy_sff_font(SFF_Descriptor_t* descriptor) {
    if(!descriptor) return;

    kfree(descriptor->global);
    kfree(descriptor->sff1);
    kfree(descriptor->image_data);
    kfree(descriptor->image_metadata);

    fclose(descriptor->file_descriptor);

    kfree(descriptor);
}