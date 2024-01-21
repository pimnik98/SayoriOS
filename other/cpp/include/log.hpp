#pragma once

#include "stdint.hpp"
#include "stdio.hpp"

namespace std {
    void qemu_puts(const char* text);

    void qemu_log(const char t);
    void qemu_log(const char* t);
    void qemu_log(const std::string t);
    void qemu_log(const void* t);
    void qemu_log(const size_t);
    void qemu_log(const ssize_t);
    void qemu_log(const EndLine);

    class QEMU_LOG {};

    template<typename T>
    std::QEMU_LOG& operator << (std::QEMU_LOG& l, const T t) {
        qemu_log(t);
        return l;
    }

    static QEMU_LOG log;
}