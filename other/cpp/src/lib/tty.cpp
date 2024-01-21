#include "tty.hpp"

extern "C" {
    void _tty_puts(const char*);
    void _tty_putint(int);
    void _tty_putuint(unsigned int);
    void _tty_puthex(size_t);
    void _tty_putchar(char, char);
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

void internals::TTY::buffered_write(const unsigned int num) {
    _tty_putuint(num);
}

void internals::TTY::buffered_write(const void* ptr) {
    _tty_puthex((size_t)ptr);
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