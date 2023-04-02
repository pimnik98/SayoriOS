#ifndef		MEMORY_H
#define		MEMORY_H

#include	"common.h"
#include	"multiboot.h"
#include	"sys/sync.h"

#define		PAGE_SIZE		0x1000	/* Memory page size */
#define		PAGE_OFFSET_BITS	12	/* */
#define		PAGE_OFFSET_MASK	0xFFF	
#define		PAGE_TABLE_INDEX_BITS	10	
#define		PAGE_TABLE_INDEX_MASK	0x3FF	

#define		PHYSADDR_BITS		32	

/*------------------------------------------------------------------------------
//		Page flags
//----------------------------------------------------------------------------*/
#define		PAGE_PRESENT		(1 << 0)
#define		PAGE_WRITEABLE		(1 << 1)
#define		PAGE_USER		(1 << 2)
#define		PAGE_WRITE_THROUGH	(1 << 3)
#define		PAGE_CACHE_DISABLE	(1 << 4)
#define		PAGE_ACCESSED		(1 << 5)
#define		PAGE_DIRTY		(1 << 6)
#define		PAGE_GLOBAL		(1 << 8)

// #define		KERNEL_BASE		0x200000		/* Kernel start address in physical memory */ // There was 0x200000 before SB16 driver
// #define		KERNEL_SIZE		0x1800000		/* Size of area for kernel */

/*------------------------------------------------------------------------------
//		User virtual address space
//----------------------------------------------------------------------------*/
// FIXME: IDK WHY IT's NEEDED
#define			USER_MEMORY_START	((void*) 0x80000000)
#define			USER_MEMORY_END		((void*) 0xFFFFFFFF)
/*------------------------------------------------------------------------------
//		Kernel virtual address space
//----------------------------------------------------------------------------*/
#define			KERNEL_MEMORY_END	((void*) 0x7FFFFFFF)

/* Kernel heap params */
#define			KERNEL_HEAP_SIZE			0x20000000
#define			KERNEL_HEAP_BASE			((void*) (KERNEL_MEMORY_END - KERNEL_HEAP_SIZE))
#define 		KERNEL_HEAP_BLOCK_INFO_SIZE	0x400000

/*------------------------------------------------------------------------------
//		Any types redefinition
//----------------------------------------------------------------------------*/
typedef		uint32_t			physaddr_t;
typedef 	uint32_t 			virtual_addr_t;
//typedef 	uint32_t 			physical_addres_t;
/*------------------------------------------------------------------------------
//		Memory pages block
//----------------------------------------------------------------------------*/
typedef	struct
{
	physaddr_t		prev;	/* Previous block pointer */
	physaddr_t		next;	/* Next block pointer */
	size_t			size;	/* Block size */

}__attribute__((packed)) physmemory_pages_block_t;

/*------------------------------------------------------------------------------
//	Memory block structure
//----------------------------------------------------------------------------*/
typedef struct
{
	void*		base;		/* Base address */
	size_t		size;		/* Memory block size */

}__attribute__((packed)) memory_block_t;

/*------------------------------------------------------------------------------
//	Kernel heap
//----------------------------------------------------------------------------*/
typedef struct
{
	memory_block_t*		blocks;			/* Memory blocks info array */
	void*			start;			/* Heap start virtual address */
	void*			end;			/* Heap end virtual address */
	size_t			size;			/* Heap size */
	size_t			count;			/* Number of allocated memory blocks */
	mutex_t			heap_mutex;		/* Sinchonization for heap access  */
	
}__attribute__((packed)) heap_t;

#define		LAST_ADDR		0xFFFFFFFF		/* Last virtual address in x86 address space*/

#define		TEMP_PAGE		(KERNEL_BASE + KERNEL_SIZE - PAGE_SIZE)	/* Temporary page for */
// #define		TEMP_PAGE		(0xFFA00000)	/* Temporary page for */
										/* physical memory access */

#define		TEMP_TABLE_IDX		(TEMP_PAGE >> 22)						/* Temporary page table index == PAGE_DIRECTORY_INDEX*/
#define		TEMP_PAGE_IDX		((TEMP_PAGE >> PAGE_OFFSET_BITS) & PAGE_TABLE_INDEX_MASK)	/* Temporary page index == PAGE_TABLE_INDEX*/

#define PAGE_ALIGN_DOWN(x)     ((x) & -PAGE_SIZE)
#define PAGE_ALIGN_UP(address) (((address) + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1) )

#define PAGE_DIRECTORY_INDEX(x) (((x) >> 22) & 0x3FF)
#define PAGE_TABLE_INDEX(x)     (((x) >> 12) & 0x3FF)

#define PAGE_GET_TABLE_ADDRESS(x)    (*x & ~0xFFF)
#define PAGE_GET_physical_addres_tESS(x) (*x & ~0xFFF)

#define GET_PDE(v) (page_dir_entry*) (0xFFFFF000 +  (v >> 22) * 4)
#define GET_PTE(v) (page_table_entry*) (0xFFC00000 + (v >> 12) * 4)

#define GET_PT() (size_t*)(kernel_page_dir + PAGE_SIZE)

/* Switch processor to page mode */
void switch_page_mode(void);
/* Check memory map from GRUB2 Multiboot header */
void check_memory_map(memory_map_entry_t* mmap_addr, uint32_t length);

void init_memory_manager(uint32_t stack);

uint8_t map_pages(physaddr_t page_dir,		/* Page directory*/
		        virtual_addr_t vaddr,		/* Start virtual address */
		        physaddr_t paddr,	/* Start physical address */
		        size_t count,		/* Size of memory space */
		        uint32_t flags);		/* Page's flags */

physaddr_t alloc_phys_pages(size_t count);
void vmm_free_page(virtual_addr_t vaddr);
physaddr_t get_kernel_dir(void);

void* kmalloc(size_t size);
void kfree(void* vaddr);
void* kcalloc(size_t count, size_t size);
void* krealloc(void* vaddr, size_t size);
size_t memory_get_used_kernel();
size_t virt2phys(physaddr_t page_directory, virtual_addr_t virtual_address);
uint8_t unmap_pages(physaddr_t page_dir, virtual_addr_t vaddr, size_t count);

#endif
