/**
 * @brief Менеджер виртуальной памяти
 * @author NDRAEY >_
 * @version 0.3.5
 * @date 2023-11-04
 * @copyright Copyright SayoriOS Team (c) 2022-2024
 */

// Charmander - a new virtual memory manager by NDRAEY (c) 2023
// for SayoriOS

#include "../../include/mem/vmm.h"
#include "../../include/mem/pmm.h"
#include "io/ports.h"

heap_t system_heap;
bool vmm_debug = false;

//size_t pmm_alloc_and_map(size_t* page_dir, size_t virtual_addr, size_t bytes) {
//	size_t count = (bytes + 1) / PAGE_SIZE;
//	size_t pages = phys_alloc_multi_pages(count);
//
//	map_pages(page_dir, pages, virtual_addr, bytes, PAGE_WRITEABLE);
//
//	return pages;
//}

size_t pmm_alloc_and_map_self(size_t* page_dir, size_t bytes) {
	size_t count = (bytes + 1) / PAGE_SIZE;
	size_t pages = phys_alloc_multi_pages(count);

	map_pages(page_dir, pages, pages, bytes, PAGE_WRITEABLE);

	return pages;
}

void vmm_init() {
    memset(&system_heap, 0, sizeof(heap_t));

	system_heap.capacity = PAGE_SIZE / sizeof(struct heap_entry);

	system_heap.allocated_count = 0;
	system_heap.used_memory = 0;

	system_heap.start = 0x1000000;
	system_heap.memory = (struct heap_entry*)pmm_alloc_and_map_self(get_kernel_page_directory(), PAGE_SIZE);

    memset(system_heap.memory, 0, PAGE_SIZE);

	qemu_log("CAPACITY: %d", system_heap.capacity);
	qemu_log("MEMORY AT: %x", (size_t)system_heap.memory);
}

void heap_dump() {
    qemu_note("Heap info: %d entries of %d possible", system_heap.allocated_count, system_heap.capacity);
    qemu_note("           %d bytes of ? bytes used", system_heap.used_memory);

    for(int i = 0; i < system_heap.allocated_count; i++) {
		qemu_log("[%d] [%x, %d => %x]",
				 i,
				 system_heap.memory[i].address,
				 system_heap.memory[i].length,
				 system_heap.memory[i].address + system_heap.memory[i].length);

		if(i < system_heap.allocated_count - 1) {
			if(system_heap.memory[i].address + system_heap.memory[i].length < system_heap.memory[i + 1].address) {
				qemu_log("FREE SPACE: %d bytes", system_heap.memory[i + 1].address - (system_heap.memory[i].address + system_heap.memory[i].length));
			}
		}
	}
}

// TODO: Handle out of memory, implement automatical heap resizing
void* alloc_no_map(size_t size, size_t align) {
	void* mem = 0;

	if(system_heap.allocated_count == 0) {
        mem = (void *) system_heap.start;

        system_heap.memory[0].address = system_heap.start;
		system_heap.memory[0].length = size;

		goto ok;
	}

	if(system_heap.allocated_count == system_heap.capacity - 1) {
		qemu_err("TODO: IMPLEMENT HEAP RESIZING!!!!");
		while(1);
	}

	for(int i = 0; i < system_heap.allocated_count; i++) {
		struct heap_entry cur = system_heap.memory[i];
		struct heap_entry next = system_heap.memory[i + 1];

		size_t curend = cur.address + cur.length;

		if(align)
			curend = ALIGN(curend, align);

		if(next.address == 0) {
			system_heap.memory[i + 1].address = curend;
			system_heap.memory[i + 1].length = size;

			mem = (void*)system_heap.memory[i + 1].address;

        	goto ok;
		} else if(curend + size <= next.address) {
			// Ok!

            for(size_t j = system_heap.allocated_count; j > i; j--) {
				system_heap.memory[j] = system_heap.memory[j - 1];
			}

			mem = (void*)(curend);
			system_heap.memory[i + 1].address = curend;
			system_heap.memory[i + 1].length = size;

			goto ok;
		}
	}

	ok:

	system_heap.allocated_count++;
	system_heap.used_memory += size;

	// end:

	return mem;
}

void free_no_map(void* ptr) {
	if(!ptr)
		return;

	size_t i = 0;
	bool found = false;

	for(; i < system_heap.allocated_count; i++) {
		if(system_heap.memory[i].address == (size_t)ptr) {
			found = true;
			break;
		}
	}

	if(!found)
		return;

	system_heap.used_memory -= system_heap.memory[i].length;

	for (; i < system_heap.allocated_count - 1; i++) {
		system_heap.memory[i] = system_heap.memory[i + 1];
	}

	system_heap.memory[i].address = 0;
	system_heap.memory[i].length = 0;

	system_heap.allocated_count--;
}

void* kmalloc_common(size_t size, size_t align) {
	void* allocated = alloc_no_map(size, align);

	if(!allocated) {
		return 0;
    }

	size_t reg_addr = (size_t) allocated & ~0xfff;

	for(size_t i = 0; i <= ALIGN(size, 4096); i += PAGE_SIZE) {
		size_t region = phys_get_page_data(get_kernel_page_directory(),
										   reg_addr); // is allocated region there?

		if (!region) {
            if(vmm_debug) {
                qemu_warn("Region is not yet mapped: %x", reg_addr);
            }

            size_t page = phys_alloc_single_page();

            if(vmm_debug) {
                qemu_log("Obtained new page: %x", page);
            }

            map_single_page(get_kernel_page_directory(),
							page,
							reg_addr,
							PAGE_WRITEABLE);

            if(vmm_debug) {
                qemu_ok("Mapped!");
            }
		} else {
            if(vmm_debug) {
                qemu_warn("Already mapped: %x (Size: %d)", reg_addr, size);
            }
		}

		reg_addr += PAGE_SIZE;
	}

    if(vmm_debug) {
    	qemu_ok("From %x to %x, here you are!", (size_t)allocated, (size_t)(allocated + size));
    }

	return allocated;
}

bool vmm_is_page_used_by_entries(size_t address) {
	for(size_t i = 0; i < system_heap.allocated_count; i++) {
		size_t start = system_heap.memory[i].address & ~0xfff;
		size_t end = ALIGN(system_heap.memory[i].address + system_heap.memory[i].length, PAGE_SIZE);

//		qemu_log("[%x => %x] %x => %x", system_heap.memory[i].address, system_heap.memory[i].address + system_heap.memory[i].length, start, end);

		if(address >= start && address < end) {
			return true;
		}
	}

	return false;
}

struct heap_entry heap_get_block(size_t address) {
	for(int i = 0; i < system_heap.allocated_count; i++) {
		if(system_heap.memory[i].address == address) {
			return system_heap.memory[i];
		}
	}

	return (struct heap_entry){};
}

// NOTE: Returns nullptr if block does not exist
struct heap_entry* heap_get_block_ref(size_t address) {
	for(int i = 0; i < system_heap.allocated_count; i++) {
		if(system_heap.memory[i].address == address) {
			return &(system_heap.memory[i]);
		}
	}

	return 0;
}

// NOTE: Returns 0xFFFFFFFF if not exist
size_t heap_get_block_idx(size_t address) {
	for(size_t i = 0; i < system_heap.allocated_count; i++) {
		if(system_heap.memory[i].address == address) {
			return i;
		}
	}

	return 0xFFFFFFFF;
}

void kfree(void* ptr) {
	if(!ptr)
		return;

	struct heap_entry block = heap_get_block((size_t)ptr);

    if(vmm_debug)
        qemu_warn("Freeing %x", (size_t)ptr);

	if(!block.address) {
		qemu_warn("No block!");
		return;
	}


	free_no_map(ptr);

	for(size_t i = 0; i < block.length; i += PAGE_SIZE) {
		if(!vmm_is_page_used_by_entries(block.address + i)) {
			size_t phys_addr = phys_get_page_data(get_kernel_page_directory(), block.address + i) & ~0xfff;

            if(vmm_debug)
    			qemu_warn("Unmapping %x => %x", block.address + i, phys_addr);

			unmap_single_page(get_kernel_page_directory(), block.address + i);

			phys_free_single_page(phys_addr);
		}
	}

//	qemu_ok("OK!");
}

void* krealloc(void* ptr, size_t memory_size) {
	if(!ptr)
		return 0;

	struct heap_entry* block = heap_get_block_ref((size_t) ptr);

	if(!block)
		return 0;

//	qemu_warn("ORIGINAL BLOCK: %x, %d", block->address, block->length);

	if(memory_size > block->length) {  // Expand
//		qemu_warn("EXPANDING FROM %d to %d", block->length, memory_size);

		size_t index = heap_get_block_idx((size_t) ptr);

		if(index == system_heap.allocated_count - 1) { // Last block?
//            qemu_log("LAST BLOCK!");

			size_t reg_addr = block->address & ~0xfff;

			for(int addr_offset = 0; addr_offset <= ALIGN(memory_size, 4096); addr_offset += PAGE_SIZE) {
				size_t region = phys_get_page_data(get_kernel_page_directory(),
												   reg_addr); // is allocated region there?

				if (!region) {
//					qemu_warn("Region is not yet mapped: %x", reg_addr);

					size_t page = phys_alloc_single_page();
//					qemu_log("Obtained new page: %x", page);

					map_single_page(get_kernel_page_directory(),
									page,
									reg_addr,
									PAGE_WRITEABLE);

//					qemu_ok("Mapped!");
				}/* else {
					qemu_warn("Already mapped: %x", reg_addr);
				}*/

                reg_addr += PAGE_SIZE;
			}

            system_heap.used_memory += memory_size - block->length;

            block->length = memory_size;
		} else {
//			qemu_err("CAN USE NEXT!");

			struct heap_entry next = system_heap.memory[index + 1];

			size_t willend = block->address + memory_size;

			if(willend < next.address) {
//				qemu_log("THERE'S FREE SPACE!");

				size_t reg_addr = block->address & ~0xfff;

				for(int addr_offset = 0; addr_offset <= ALIGN(memory_size, 4096); addr_offset += PAGE_SIZE) {
					size_t region = phys_get_page_data(get_kernel_page_directory(),
													   reg_addr); // is allocated region there?

					if (!region) {
//						qemu_warn("Region is not yet mapped: %x", reg_addr);

						size_t page = phys_alloc_single_page();
//						qemu_log("Obtained new page: %x", page);

						map_single_page(get_kernel_page_directory(),
										page,
										reg_addr,
										PAGE_WRITEABLE);

//						qemu_ok("Mapped!");
					}/* else {
						qemu_warn("Already mapped: %x", reg_addr);
					}*/

					reg_addr += PAGE_SIZE;
				}

                system_heap.used_memory += memory_size - block->length;

				block->length = memory_size;
			} else {
//				qemu_err("No space between blocks! :(");  // IT'S NORMAL

				void* new_block = kmalloc(memory_size);

				memcpy(new_block, (const void *) block->address, block->length);

				kfree(ptr);

				return new_block;
			}

//			qemu_ok("Next is %x, %d", next.address, next.length);
		}
	} else if(memory_size < block->length) {  // Shrink
		qemu_warn("SHRINKING FROM %d to %d", block->length, memory_size);

        system_heap.used_memory -= block->length - memory_size;

		block->length = memory_size;
	}

	return (void *) block->address;
}

/**
 * @brief Копирует адресное пространство ядра в новое адресное пространство
 * @return Виртуальный адрес директории страниц нового адресного пространства
 */
void* clone_kernel_page_directory(size_t virts_out[1024]) {
	uint32_t* page_dir = kmalloc_common(PAGE_SIZE, PAGE_SIZE);
    memset(page_dir, 0, PAGE_SIZE);

    uint32_t physaddr = virt2phys(get_kernel_page_directory(), (virtual_addr_t) page_dir);

    const uint32_t* kern_dir = get_kernel_page_directory();
    const uint32_t linaddr = (const uint32_t)(page_directory_start);

//    uint32_t* addresses[1024] = {0};

    for(int i = 0; i < 1023; i++) {
        if (kern_dir[i]) {
            uint32_t *page_table = kmalloc_common(PAGE_SIZE, PAGE_SIZE);

            virts_out[i] = (size_t)page_table;
        }
    }

    for(int i = 0; i < 1023; i++) {
        if (kern_dir[i]) {
            uint32_t* page_table = (uint32_t*)virts_out[i];
            uint32_t physaddr_pt = virt2phys(kern_dir, (virtual_addr_t) page_table);

            qemu_log("Copying from %x to %x", linaddr + (i * PAGE_SIZE), (size_t)page_table);

            memcpy(page_table, (void*)(linaddr + (i * PAGE_SIZE)), PAGE_SIZE);

            for(int j = 0; j < 1024; j++) {
                page_table[j] = (page_table[j] & ~(PAGE_DIRTY | PAGE_ACCESSED));
            }

            page_dir[i] = physaddr_pt | 3;
        }
    }

    page_dir[1023] = physaddr | 3;

    for(int i = 0; i < 1024; i++)
        if(page_dir[i])
            qemu_log("[%d] %x = %x", i, kern_dir[i], page_dir[i]);

    qemu_log("Page directory at: V%x (P%x); Here you are!", (size_t)page_dir, physaddr);

	return page_dir;
}

void vmm_debug_switch(bool enable) {
    vmm_debug = enable;
}
