#pragma once

#include <kernel.h>

extern bool stateTTY;

void _tty_puts(const char str[]);
void _tty_print(const char *format, va_list args);
void _tty_printf(char *text, ...);

#define tty_print(format, args) _tty_print(format, args); punch()
#define tty_puts(str) _tty_puts(str); punch()

static inline void tty_printf(char *text, ...) {
    if (stateTTY){
        va_list args;
        va_start(args, text);
        tty_print(text, args);
        va_end(args);
    }
}

void drawRect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t color);
void setPosX(uint32_t x);
void setPosY(uint32_t y);

void tty_setcolor(uint32_t color);
uint32_t tty_getcolor();

void tty_changeState(bool state);
void tty_set_bgcolor(uint32_t color);

void set_cursor_enabled(bool en);

void _tty_putuint(const size_t i);
void _tty_putint(const ssize_t i);
void _tty_putchar(char c, char c1);
void tty_putchar(char c, char c1);

void clean_screen();
void buffer_set_pixel4(uint8_t *buffer, size_t width, size_t height, size_t x, size_t y, size_t color);