#include "common.h"

// Prints a hexedecimal dump of memory.
void hexview_advanced(char* buffer, size_t length, size_t width, bool relative, void (*printer_func)(const char *, ...)) {
    for(size_t i = 0; i < length; i += width) {
        if(relative)
            printer_func("%08v: ", i);
        else
            printer_func("%08v: ", buffer + i);

        for(int j = 0; j < (length - i < width ? length - i : width); j++) {
            printer_func("%02v ", ((char)*(buffer + i + j)) & 0xFF);
        }

        printer_func("\n");
    }
}