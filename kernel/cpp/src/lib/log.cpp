#include "log.hpp"
#include "conv.hpp"

#define PORT_COM1 0x3f8

extern "C" {
    void __com_writeString(uint16_t port, const char *buf);
    void __com_writeChar(uint16_t port, char a);
    void __com_writeHex(int16_t port, uint32_t i, bool mode);
}

void std::qemu_puts(const char* text) {
    __com_writeString(PORT_COM1, text);
}

void std::qemu_log(const char t) {
    __com_writeChar(PORT_COM1, t);
}

void std::qemu_log(const char* t) {
    qemu_puts(t);
}

void std::qemu_log(std::string t) {
    qemu_puts(t.c_str);
}

void std::qemu_log(const void* t) {
    __com_writeHex(PORT_COM1, (size_t)t, true);
}

void std::qemu_log(const size_t t) {
    std::string* temp = std::itoa(t, 10);

    qemu_log(temp->c_str);

    delete temp;
}

void std::qemu_log(const ssize_t t) {
    std::string* temp = std::itoa(t, 10);

    qemu_log(temp->c_str);

    delete temp;
}