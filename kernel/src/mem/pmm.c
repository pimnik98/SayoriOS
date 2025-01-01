/**
 * @brief Менеджер физической памяти
 * @author NDRAEY >_
 * @version 0.3.5
 * @date 2023-11-04
 * @copyright Copyright SayoriOS Team (c) 2022-2024
 */

// Scyther Physical Memory Manager by NDRAEY (c) 2023
// for SayoriOS

#include <common.h>
#include "lib/math.h"
#include "mem/pmm.h"
#include "lib/string.h"
#include "io/ports.h"

extern size_t KERNEL_BASE_pos;
extern size_t KERNEL_END_pos;

size_t kernel_start;
size_t kernel_end;

size_t mmap_length = 0;
memory_map_entry_t*	mentry = 0;

size_t phys_memory_size = 0;
size_t used_phys_memory_size = 0;

physical_addr_t* kernel_page_directory = 0;

// Create 128 KB page bitmap for saving data about 4 GB space.
/// Карта занятых страниц
uint8_t pages_bitmap[PAGE_BITMAP_SIZE] = {0};

bool paging_initialized = false;

/**
 * @brief Выделяет одну страницу физической памяти (4096 байт)
 * @return Физический адрес страницы
 */
physical_addr_t phys_alloc_single_page() {
	if(used_phys_memory_size >= phys_memory_size) {
		// If no free space, just call the function that handles that situation.
		qemu_log("No free physical memory. Running emergency scenario...");

		phys_not_enough_memory();
	}

	for(int i = 0; i < PAGE_BITMAP_SIZE; i++) {
		if(pages_bitmap[i] == 0xff) {
			// 0xff is eight ones in 8 bit.
			// 8 ones - all pages in this index is used.
			continue;
		} else {
			// Otherwise, we have some bits cleared - we have free page(s).
			// Roll over all bits
			for(int j = 0; j < 8; j++) {
				// If we have bit cleared, mark it as used, increment memory stat and return address.
				if(((pages_bitmap[i] >> j) & 1) == 0) {
					// Page is free
					pages_bitmap[i] |= (1 << j);

					used_phys_memory_size += PAGE_SIZE;

					// Every (8bit) entry can handle (4096 * 8) = 32768 bytes.
					// Every bit of entry can hold one page (4096 bytes). 
					return (PAGE_SIZE * 8 * i) + (j * PAGE_SIZE);
				}
			}
		}
	}

	return 0;
}

/**
 * @brief Выделяет несколько страниц последовательно
 * @param count количество страниц
 * @return Физический адрес где начинаются страницы
 */
physical_addr_t phys_alloc_multi_pages(size_t count) {
	if(used_phys_memory_size + (count * PAGE_SIZE) >= phys_memory_size) {
		qemu_log("No free physical memory. Running emergency scenario...");

		phys_not_enough_memory();
	}

	size_t counter = 0;
	size_t addr = 0;

	// They used for saving start indexes of our pages.
	size_t si, sj = 0;

	for(int i = 0; i < PAGE_BITMAP_SIZE; i++) {
		if(pages_bitmap[i] == 0xff) {
			// 0xff is eight ones in 8 bit.
			// 8 ones - all pages in this index is used.
			continue;
		} else {
			// Otherwise, we have some bits cleared - we have free page(s).
			// Roll over all bits
			for(int j = 0; j < 8; j++) {
				// Check if page is free
				if(((pages_bitmap[i] >> j) & 1) == 0) {
					// If we starting, we need to save an address.
					if(counter == 0) {
						si = i;
						sj = j;
						addr = (PAGE_SIZE * 8 * i) + (j * PAGE_SIZE);
					} else if(counter == count) {
						// If we found `count` free pages in a row, we should mark them as used and return address.

						// Roll through all entries, starting from indices we preserved.
						for(; si < PAGE_BITMAP_SIZE; si++) {
							for(; sj < 8; sj++) {
								// Mark as used.
								pages_bitmap[si] |= (1 << sj);

								// We have no control, so keep loops running until we mark all `count` pages as used.
								// If we marked all pages, exit the loops.
								if(!--counter)
									goto phys_multialloc_end;
							}

							sj = 0;
						}

						phys_multialloc_end:

						used_phys_memory_size += PAGE_SIZE * count;

						return addr;
					}

					// We found a free page, so increment a counter
					counter++;
				} else {
					// Oh shit, we have encountered a used page! (Loud scream!)
					// Okay, just reset the counter and address to start from the beginning.

					counter = 0;
					addr = 0; // For sanity
				}
			}
		}
	}

	return 0;
}

/**
 * @brief Освобождает страницу физической памяти
 * @param addr Физический адрес страницы
 */
void phys_free_single_page(physical_addr_t addr) {
	if(!addr)
		return;

	// Extract our entry position (i) and bit in the entry (j).

	size_t i = addr / (PAGE_SIZE * 8);
	size_t j = (addr % (PAGE_SIZE * 8)) / PAGE_SIZE;

	// Just clear a nth bit
	pages_bitmap[i] &= ~(1 << j);

	used_phys_memory_size -= PAGE_SIZE;
}

/**
 * @brief освобождает несколько страниц подряд
 * @param addr Физический адрес где начинаются страницы
 * @param count Количество страниц
 */
void phys_free_multi_pages(physical_addr_t addr, size_t count) {
	if(!addr)
		return;

	// Extract our entry position (i) and bit in the entry (j).

	size_t i = addr / (PAGE_SIZE * 8);
	size_t j = (addr % (PAGE_SIZE * 8)) / PAGE_SIZE;

	// Roll over all entries starting from index of our address
	for(; i < PAGE_BITMAP_SIZE; i++) {
		for(; j < 8; j++) {
			// Just clear a nth bit
			pages_bitmap[i] &= ~(1 << j);
		
			// If we freed all pages, just exit the function.
			if(!--count) {
				return;
			}
		}

		j = 0;
	}

	used_phys_memory_size -= PAGE_SIZE * count;
}

// Tells if page allocated there
bool phys_is_used_page(physical_addr_t addr) {
	if(!addr)
		return true;

	// Extract our entry position (i) and bit in the entry (j).

	size_t i = addr / (PAGE_SIZE * 8);
	size_t j = (addr % (PAGE_SIZE * 8)) / PAGE_SIZE;

	// Just clear a nth bit
	return (bool)((pages_bitmap[i] >> j) & 1);
}

// Marks page.
void phys_mark_page_entry(physical_addr_t addr, uint8_t used) {
	if(!addr)
		return;

	// Extract our entry position (i) and bit in the entry (j).

	size_t i = addr / (PAGE_SIZE * 8);
	size_t j = (addr % (PAGE_SIZE * 8)) / PAGE_SIZE;

	if(used)
		pages_bitmap[i] |= (1 << j);
	else
		pages_bitmap[i] &= ~(1 << j);
}

// Creates and prepares a page directory
uint32_t * new_page_directory() {
	// Allocate a page (page directory is 4096 bytes)
	uint32_t* dir = (uint32_t*)phys_alloc_single_page();

	qemu_log("Allocated page directory at: %x", dir);

	// Blank it (they can store garbage, so we need to blank it)
	memset(dir, 0, 4096);

//	uint32_t page_table = phys_alloc_single_page();
//
//	dir[PD_INDEX((uint32_t)dir)] = page_table | 3;
//
//	((uint32_t*)page_table)[PT_INDEX((uint32_t)dir)] = (uint32_t)dir | 3;

	dir[1023] = (uint32_t)dir | 3;

//	for(int i = 0; i < 1024; i++) {
//		uint32_t addr = phys_alloc_single_page();
//		uint32_t* pt = (uint32_t*)addr;
//
//		for(int j = 0; j < 1024; j++) {
//			pt[j] = 0;
//		}
//
//		dir[i] = addr | 3;
//
//		((uint32_t*)(dir[PD_INDEX(addr)] & ~0x3ff))[PT_INDEX(addr)] = addr | 3;
//	}

	qemu_log("Blanked directory.");
//	qemu_log("================ Mapping a page directory");

	// Self-map page
//	map_single_page(dir, (uint32_t)dir, (uint32_t)dir, PAGE_WRITEABLE);

	qemu_log("================ Page directory is ready.");

	return dir;
}

void blank_page_directory(uint32_t* pagedir_addr) {
	// Just roll over 1024 entries and set them to 0.
	for (size_t i = 0; i < 1024; i++) {
		pagedir_addr[i] = 0;  // Fully blank
	}
}

uint32_t* get_page_table_by_vaddr(uint32_t* page_dir, virtual_addr_t vaddr) {
	if(paging_initialized)
		return (uint32_t*)((char*)page_directory_start + (PD_INDEX(vaddr) * PAGE_SIZE));
	else
		return (uint32_t*)(page_dir[PD_INDEX(vaddr)] & ~0xfff);
}

void reload_cr3() {
	__asm__ volatile("mov %cr3, %eax\n"
		"mov %eax, %cr3");
}

// Maps a page.
// Note: No need to set PAGE_PRESENT flag, it sets automatically.
// TODO: Rewrite this, this is buggy

/// \brief Maps physical page with virtual address space
/// \param page_dir VIRTUAL address of page directory
/// \param physical PHYSICAL address to map
/// \param virtual VIRTUAL address to map
/// \param flags Page flags (PAGE_PRESENT is automatically included)
void map_single_page(physical_addr_t* page_dir, physical_addr_t physical, virtual_addr_t virtual, uint32_t flags) {
	// Clean flags and some garbage from addresses.

	virtual &= ~0xfff;
	physical &= ~0xfff;

//	qemu_log("V%x => P%x", virtual, physical);

	// Get our Page Directory Index and Page Table Index.
	uint32_t pdi = PD_INDEX(virtual);
	uint32_t pti = PT_INDEX(virtual);

	uint32_t* pt;

	// Check if page table not present.
	if((page_dir[pdi] & 1) == 0) {
		pt = (uint32_t *)phys_alloc_single_page();

		page_dir[pdi] = (uint32_t)pt | PAGE_WRITEABLE | PAGE_PRESENT;

		if(paging_initialized && page_dir == get_kernel_page_directory()) {
			uint32_t pt_addr = (uint32_t)page_directory_start + (pdi * PAGE_SIZE);

			memset((uint32_t*)pt_addr, 0, PAGE_SIZE);

			pt = (uint32_t*)pt_addr;
		} else if(paging_initialized && page_dir != get_kernel_page_directory()) {
			qemu_warn("FIXME: Mapping other page directories");
			while(1);
		} else {
            memset(pt, 0, PAGE_SIZE);
		}
	} else {
		// Just get our page table
		pt = get_page_table_by_vaddr(page_dir, virtual);
	}

//	qemu_log("P: %x | V: %x => PDI: %d | PTI: %d", physical, virtual, pdi, pti);

	// Finally map our physical page to virtual
	pt[pti] = physical | flags | PAGE_PRESENT;

	// Do our best to take effect.
	reload_cr3();
	__asm__ volatile ("invlpg (,%0,)"::"a"(virtual));
}

void unmap_single_page(uint32_t* page_dir, virtual_addr_t virtual) {
	virtual &= ~0xfff;
	
	uint32_t* pt;
	
	// Check if page table not present.
	if((page_dir[PD_INDEX(virtual)] & 1) == 0) {
		return;
	} else {
//		qemu_log("Page table exist");
		pt = get_page_table_by_vaddr(page_dir, virtual);
	}

//	qemu_log("Got page table at: %x", pt);
//	qemu_log("Unmapping: %x", virtual);

	pt[PT_INDEX(virtual)] = 0;

	reload_cr3();
}

uint32_t phys_get_page_data(uint32_t* page_dir, virtual_addr_t virtual) {
	virtual &= ~0x3ff;
	
	uint32_t* pt;
	
	// Check if page table not present.
	if((page_dir[PD_INDEX(virtual)] & 1) == 0) {
		return 0;
	} else {
		pt = get_page_table_by_vaddr(page_dir, virtual);
	}

	return pt[PT_INDEX(virtual)];
}

uint32_t virt2phys(const uint32_t *page_dir, virtual_addr_t virtual) {
//	virtual &= ~0x3ff;
	virtual &= ~0xfff;

	uint32_t* pt;
	
	// Check if page table not present.
	if((page_dir[PD_INDEX(virtual)] & 1) == 0) {
		return 0;
	} else {
		pt = get_page_table_by_vaddr(page_dir, virtual);
	}

	return pt[PT_INDEX(virtual)] & ~0x3ff;
}

void phys_set_flags(uint32_t* page_dir, virtual_addr_t virtual, uint32_t flags) {
    virtual &= ~0xfff;

    uint32_t* pt;

    // Check if page table not present.
    if((page_dir[PD_INDEX(virtual)] & 1) == 0) {
        return;
    } else {
        pt = get_page_table_by_vaddr(page_dir, virtual);
    }

    pt[PT_INDEX(virtual)] = (pt[PT_INDEX(virtual)] & ~0x3ff) | flags | PAGE_PRESENT;
}


/**
 * @brief Map pages
 *
 * @param page_dir Page directory address
 * @param physical Address of physical memory
 * @param virtual Address of virtual memory
 * @param size Amount of BYTES to map (must be aligned by 4096)
 * @param flags Page flags
 */
void map_pages(uint32_t* page_dir, physical_addr_t physical, virtual_addr_t virtual, size_t size, uint32_t flags) {	
	physical_addr_t phys = physical;
	physical_addr_t virt = virtual;
	
	virtual_addr_t vend = ALIGN(virt + size, PAGE_SIZE);

	for(;
		virt <= vend;
		phys += PAGE_SIZE,
		virt += PAGE_SIZE
		) {
			map_single_page(page_dir, phys, virt, flags);
	}

	reload_cr3();
}

void check_memory_map(memory_map_entry_t* mmap_addr, uint32_t length){
	int i;
	/* Entries number in memory map structure */
	mmap_length = length;
	size_t n = length / sizeof(memory_map_entry_t);

	/* Set pointer to memory map */
	mentry = mmap_addr;
	qemu_log("[PMM] Map:");
	for (i = 0; i < n; i++){
		memory_map_entry_t entry = mentry[i];
		
		qemu_log("%s [Address: %x | Length: %x] <%d>",
				 (entry.type == 1?"Available":"Reserved"),
				 entry.addr_low, entry.len_low, entry.type);

		phys_memory_size += entry.len_low;
	}

	qemu_log("RAM: %d MB | %d KB | %d B", phys_memory_size/(1024*1024), phys_memory_size/1024, phys_memory_size);
}

size_t getInstalledRam(){
    return phys_memory_size;
}

void mark_reserved_memory_as_used(memory_map_entry_t* mmap_addr, uint32_t length){
	int i;
	/* Entries number in memory map structure */
	mmap_length = length;
	size_t n = length / sizeof(memory_map_entry_t);

	/* Set pointer to memory map */
	mentry = mmap_addr;
	for (i = 0; i < n; i++){
		memory_map_entry_t entry = mentry[i];

		size_t addr = entry.addr_low;
		size_t length = entry.len_low;

		if(entry.type != 1) {
			for(int j = 0; j < length; j += PAGE_SIZE) {
				phys_mark_page_entry(addr + j, 1);  // Mark as used
			}
		}
	}

	qemu_log("RAM: %d MB | %d KB | %d B", phys_memory_size/(1024*1024), phys_memory_size/1024, phys_memory_size);
}

uint32_t* get_kernel_page_directory() {
	if(paging_initialized)
		return page_directory_virt;
	else
		return kernel_page_directory;
}

void init_paging() {
    extern size_t grub_last_module_end;

    qemu_log("Memory bitmap covers: %d MB", (sizeof(pages_bitmap) * 8) >> 10);

	kernel_start = (size_t)&KERNEL_BASE_pos;
	kernel_end = (size_t)&KERNEL_END_pos;

    qemu_log("MODEND: %x; &MODEND: %x", grub_last_module_end, (size_t)&grub_last_module_end);

	size_t real_end = (size_t)grub_last_module_end;

	size_t kernel_size = real_end - kernel_start;

	qemu_log("Kernel starts at: %x", kernel_start);
	qemu_log("Kernel ends   at: %x (only kernel)", kernel_end);
	qemu_log("Kernel ends   at: %x (everything)", real_end);

	qemu_log("Kernel size is: %d (%d kB) (%d MB)", (kernel_end - kernel_start), (kernel_end - kernel_start) >> 10, (kernel_end - kernel_start) >> 20);
	qemu_log("Kernel size (initrd included) is: %d (%d kB) (%d MB)", kernel_size, kernel_size >> 10, kernel_size >> 20);

	kernel_size = ALIGN(kernel_size, PAGE_SIZE);

	// Preallocate our kernel space

	qemu_log("Allocating %d pages for kernel space...", (real_end / 4096) + 1);

	for(int i = 0; i < (real_end / 4096) + 1; i++) {
		// Note: if allocator returns 0, it's an error.
		// But we don't care, because we're allocating pages for first time here.
		phys_alloc_single_page();
	}

	// Create new page directory

	kernel_page_directory = (physical_addr_t*)new_page_directory();

	qemu_log("New page directory at: %x", kernel_page_directory);

	map_pages(kernel_page_directory, 0, 0, ALIGN(real_end, PAGE_SIZE), PAGE_WRITEABLE);

	qemu_log("Max: %x", phys_memory_size);

	// while(1);
	load_page_directory((size_t) kernel_page_directory);

	qemu_log("Ok?");

	enable_paging();

	paging_initialized = true;

	// Here paging enabled and every memory error will lead to a Page Fault

	uint32_t* pd = get_kernel_page_directory();

	for(int i = 0; i < 1024; i++) {
		if(pd[i] != 0)
			qemu_log("[%d]: %x", i, pd[i]);
	}
}


/**
 * @brief Map pages (but physical address can be unaligned)
 *
 * @param page_dir Page directory address
 * @param physical Address of physical memory
 * @param virtual Address of virtual memory
 * @param size Amount of BYTES to map (can be without align, if you want)
 * @param flags Page flags
 */
void map_pages_overlapping(physical_addr_t* page_directory, size_t physical_start, size_t virtual_start, size_t size, uint32_t flags) {
    // Explanation: We want to map address 0xd000abcd with size 2345
    // If we will use map_pages it will map only one page, because addresses gets aligned to PAGE_SIZE
    // (0xd000abcd -> 0xd000a000), and size too (2345 -> 4096)
    // So it uses memory from 0xd000abcd to 0xd000b4f6 (2 pages)
    //
    // We need to calculate how many pages we need to map
//    size_t pages_to_map = (size + PAGE_SIZE - 1) / PAGE_SIZE;
    // And then map them

    size_t nth1 = virtual_start / PAGE_SIZE;
    size_t nth2 = (virtual_start + size) / PAGE_SIZE;

    size_t pages_to_map = (nth2 - nth1) + 1;

    qemu_log("Range: %x - %x", virtual_start, virtual_start + size);

    qemu_note("Mapping %u pages to %x", pages_to_map, physical_start);
    map_pages(page_directory, physical_start, virtual_start, pages_to_map * PAGE_SIZE, flags);
}

void unmap_pages_overlapping(physical_addr_t* page_directory, size_t virtual, size_t size) {
//    size_t pages_to_map = (size + PAGE_SIZE - 1) / PAGE_SIZE;
    virtual &= ~0xfff;

    size_t nth1 = virtual / PAGE_SIZE;
    size_t nth2 = (virtual + size) / PAGE_SIZE;

    size_t pages_to_map = (nth2 - nth1) + 1;

    for(size_t i = 0; i < pages_to_map; i++) {
        unmap_single_page(page_directory, virtual + (i * PAGE_SIZE));
    }
}

void phys_not_enough_memory() {
	qemu_log("Not enough memory!");

	while(1);
}
