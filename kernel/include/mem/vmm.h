// Charmander - a new virtual memory manager by NDRAEY (c) 2023
// for SayoriOS
//
// Created by ndraey on 05.11.23.
//

#pragma once

#include <common.h>
#include "lib/string.h"

// #define kfree_debug(ptr) do { qemu_note("FREE BLOCK (%x) at %s:%d", ptr, __FILE__, __LINE__); kfree(ptr); } while(0);

struct heap_entry {
	size_t address;
	size_t length;
};

typedef struct heap {
	size_t allocated_count;
	size_t capacity; // Entries
	size_t start;
	size_t used_memory;
	struct heap_entry* memory;
} heap_t;

extern heap_t system_heap;

void vmm_init();
void *alloc_no_map(size_t size, size_t align);
void free_no_map(void* ptr);
bool vmm_is_page_used_by_entries(size_t address);
void* kmalloc_common(size_t size, size_t align);

SAYORI_INLINE void* kmalloc(size_t size) {
//	return kmalloc_common(size, sizeof(size_t));  // Alignment, blyad
	return kmalloc_common(size, 0);
}

void* krealloc(void* ptr, size_t memory_size);
void kfree(void* ptr);
void* clone_kernel_page_directory();

void vmm_debug_switch(bool enable);

SAYORI_INLINE void* kcalloc(size_t size, size_t amount) {
	void* x = kmalloc(size * amount);

	memset(x, 0, size * amount);

	return x;
}
