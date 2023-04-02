#include "tty.hpp"

extern "C" {
    void _tty_puts(const char*);
    void _tty_putint(int);
    void _tty_putchar(char, char);
    void punch();
}

void internals::TTY::buffered_write(const char* bytes) {
    _tty_puts(bytes);
}

void internals::TTY::buffered_write(char character1, char character2) {
    _tty_putchar(character1, character2);
}

void internals::TTY::buffered_write(const int num) {
    _tty_putint(num);
}

void internals::TTY::buffered_write(double num, size_t after_dot) {
    if(num < 0) {
        num = -num;
        _tty_puts("-");
    }

    float rem = num - (int)num;

    _tty_putint((int)num);
    _tty_puts(".");
    
    for(size_t n = 0; n < after_dot; n++) {
        _tty_putint((int)(rem * math::ipow(10, n+1)) % 10);
    }
}

void internals::TTY::buffer_flush() { punch(); }

void internals::TTY::write(const char* bytes) {
    buffered_write(bytes);
    punch();
}

void internals::TTY::write(char character1, char character2) {
    buffered_write(character1, character2);
    punch();
}

void internals::TTY::write(const int num) {
    buffered_write(num);
    punch();
}

void internals::TTY::write(double num) {
    buffered_write(num);
    punch();
}

void internals::TTY::write(const std::string& str) {
    buffered_write(str.c_str);
    punch();
}