#include "memory.hpp"

extern "C" {
    void* memcpy(void *destination, const void *source, size_t n);
    void* memset(void* ptr, uint8_t value, size_t size);
    void* memmove(void *dest, void *src, size_t count);

    void* kmalloc(size_t count);
    void kfree(void* ptr);
}

// void* memory::copy(char* destination, const char* source, size_t n) {
//     memcpy(destination, source, n);
// }

void* memory::alloc(size_t count) { return kmalloc(count); }

void  memory::free(void* ptr) { kfree(ptr); }

void operator delete[](void* ptr, size_t size) noexcept {
    memory::free(ptr);
}

void operator delete[](void* ptr) noexcept {
    memory::free(ptr);
}

void* operator new[](size_t sz) noexcept {
    if (sz == 0)
        ++sz;
        
    if (void *ptr = memory::alloc(sz))
        return ptr;
}

void* operator new(size_t sz) noexcept {
    if(sz == 0) sz++;

    return memory::alloc(sz);
}

void operator delete(void* ptr) noexcept {
    memory::free(ptr);
}

void operator delete(void* ptr, size_t size) noexcept {
    memory::free(ptr);
}
