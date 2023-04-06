#pragma once

#include "stdint.hpp"

extern "C" void* memcpy(void *destination, const void *source, size_t n);
extern "C" void* memset(void* ptr, const uint8_t value, size_t count);

namespace memory {

    template <typename T>
    void* copy(T* destination, const T* source, size_t n) {
        memcpy(destination, source, sizeof(T) * n);
    }

    template <typename T>
    void* set(T* destination, const T value, size_t n) {
        memset(destination, value, sizeof(T) * n);
    }

    void* alloc(size_t count);

    void  free(void* ptr);
}

void* operator new(size_t sz) noexcept;
void* operator new[](size_t sz) noexcept;

void operator delete(void* ptr) noexcept;
void operator delete(void* ptr, size_t size) noexcept;
void operator delete[](void* ptr) noexcept; 
void operator delete[](void* ptr, size_t size) noexcept;