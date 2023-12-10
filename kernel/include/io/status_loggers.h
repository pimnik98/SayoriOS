#pragma once

#define COLOR_TEXT      0xFFFFFF
#define COLOR_SYS_TEXT  0x92D7D4
#define COLOR_SYS_PATH  0x335190
#define COLOR_ERROR     0xA5383B
#define COLOR_ALERT     0xA66938
#define COLOR_ATENTION  0xDBF03E
#define COLOR_BG        0x000000

#include <kernel.h>
#include <io/ports.h>
#include <drv/beeper.h>
#include <io/status_loggers.h>
#include <io/status_sounds.h>
#include <io/tty.h>

void tty_error(char* format, ...);
void tty_attention(char* format, ...);
void tty_alert(char* format, ...);
void tty_global_error(char* format, ...);