#pragma once

#define COLOR_ERROR     0xA5383B
#define COLOR_ALERT     0xA66938
#define COLOR_ATENTION  0xDBF03E
#define COLOR_BG        0x000000

#include <io/tty.h>

void tty_error(char* format, ...);
void tty_attention(char* format, ...);
void tty_alert(char* format, ...);
void tty_global_error(char* format, ...);