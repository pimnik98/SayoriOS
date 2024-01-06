//
// Created by maractus on 27.12.23. (I thought it decides from my GitHub username, but not hahaahahah)
//

#include "lib/utf_conversion.h"

/// Сложная фигня с Unicode которую мне придется когда-нибудь изучить.
void utf16_to_utf8(const unsigned short *utf16, int utf16_length, char *utf8) {
    int i = 0;
    int j = 0;

    while (i < utf16_length) {
        unsigned int codepoint;

        if (utf16[i] < 0xD800 || utf16[i] > 0xDFFF) {
            // Простой BMP символ (не суррогатная пара)
            codepoint = utf16[i];
            i++;
        } else {
            // Суррогатная пара
            unsigned short high = utf16[i];
            unsigned short low = utf16[i + 1];
            codepoint = 0x10000 + ((high - 0xD800) << 10) + (low - 0xDC00);
            i += 2;
        }

        // Преобразование кодовой точки в UTF-8
        if (codepoint < 0x80) {
            utf8[j++] = (unsigned char)codepoint;
        } else if (codepoint < 0x800) {
            utf8[j++] = 0xC0 | ((codepoint >> 6) & 0x1F);
            utf8[j++] = 0x80 | (codepoint & 0x3F);
        } else if (codepoint < 0x10000) {
            utf8[j++] = 0xE0 | ((codepoint >> 12) & 0x0F);
            utf8[j++] = 0x80 | ((codepoint >> 6) & 0x3F);
            utf8[j++] = 0x80 | (codepoint & 0x3F);
        } else {
            utf8[j++] = 0xF0 | ((codepoint >> 18) & 0x07);
            utf8[j++] = 0x80 | ((codepoint >> 12) & 0x3F);
            utf8[j++] = 0x80 | ((codepoint >> 6) & 0x3F);
            utf8[j++] = 0x80 | (codepoint & 0x3F);
        }
    }
}
