#pragma once

// Scyther Physical Memory Manager by NDRAEY (c) 2023
// for SayoriOS

#include	"common.h"
#include	"multiboot.h"

typedef uint32_t pt_entry;
typedef uint32_t pd_entry;

extern size_t phys_memory_size;
extern size_t used_phys_memory_size;

extern size_t kernel_start;
extern size_t kernel_end;

#define		PAGE_SIZE				0x1000
#define		PAGE_OFFSET_MASK		0xFFF	
#define		PAGE_TABLE_INDEX_BITS	10	
#define		PAGE_TABLE_INDEX_MASK	0x3FF

#define		PAGE_PRESENT		(1 << 0)
#define		PAGE_WRITEABLE		(1 << 1)
#define		PAGE_USER			(1 << 2)
#define		PAGE_WRITE_THROUGH	(1 << 3)
#define		PAGE_CACHE_DISABLE	(1 << 4)
#define		PAGE_ACCESSED		(1 << 5)
#define		PAGE_DIRTY			(1 << 6)
#define		PAGE_GLOBAL			(1 << 8)

#define     PAGE_BITMAP_SIZE (131072)

#define 	PD_INDEX(virt_addr) ((virt_addr) >> 22)
#define 	PT_INDEX(virt_addr) (((virt_addr) >> 12) & 0x3ff)

typedef 	uint32_t 			virtual_addr_t;
typedef 	uint32_t 			physical_addr_t;

// The space where first page table starts
/// Начало где расположены все таблицы страниц
static uint32_t* page_directory_start = (uint32_t*)(0xffffffff - (4 * MB) + 1);

// The space where we can modify page directory
/// Начало директории таблиц для страниц
static uint32_t* page_directory_virt = (uint32_t*)(0xffffffff - (4 * KB) + 1);


extern void load_page_directory(size_t addr);
extern void enable_paging();

physical_addr_t phys_alloc_single_page();
physical_addr_t phys_alloc_multi_pages(size_t count);
void phys_free_single_page(physical_addr_t addr);
void phys_free_multi_pages(physical_addr_t addr, size_t count);
void map_single_page(physical_addr_t* page_dir, physical_addr_t physical, virtual_addr_t virtual, uint32_t flags);
void unmap_single_page(uint32_t* page_dir, virtual_addr_t virtual);
void map_pages(uint32_t* page_dir, physical_addr_t physical, virtual_addr_t virtual, size_t size, uint32_t flags);
void phys_not_enough_memory();
void blank_page_directory(uint32_t* pagedir_addr);
bool phys_is_used_page(physical_addr_t addr);
void phys_mark_page_entry(physical_addr_t addr, uint8_t used);
uint32_t phys_get_page_data(uint32_t* page_dir, virtual_addr_t virtual);
uint32_t virt2phys(uint32_t* page_dir, virtual_addr_t virtual);
void init_paging();
uint32_t* get_kernel_page_directory();

void map_pages_overlapping(physical_addr_t* page_directory, size_t physical_start, size_t virtual_start, size_t size, uint32_t flags);
void unmap_pages_overlapping(physical_addr_t* page_directory, size_t virtual, size_t size);