/**
 * @file sys/memory.c
 * @author Пиминов Никита (nikita.piminoff@yandex.ru)
 * @brief Менеджер памяти
 * @version 0.3.0
 * @date 2022-10-01
 * @copyright Copyright SayoriOS Team (c) 2022
 */
#include		"sys/memory.h"
#include		"drv/text_framebuffer.h"
#include		"io/ports.h"
#include		"lib/string.h"

physaddr_t kernel_page_dir = KERNEL_PAGE_TABLE;	///< Адрес каталога страницы ядра
memory_map_entry_t*	mentry = 0;					///< ...
uint64_t phys_memory_size = 0;					///< Всего ОЗУ
size_t free_pages_count = 0;					///< Количество свободных страниц
physaddr_t free_phys_memory_pointer = -1;		///< Свободная ячейка ОЗУ
uint32_t kernel_stack = 0;						///< Точка входа
uint32_t memory_size = 0;						///< ...
heap_t kheap;									///< ...
mutex_t phys_memory_mutex;						///< ...

/**
 * @brief Переключение режима страничной памяти
 */
void switch_page_mode(void){
	uint64_t		vaddr = 0;
	uint32_t		frame = 0;
	physaddr_t	paddr = 0;
	uint32_t		table_idx = 0;
	uint32_t		page_idx = 0;
	uint32_t		cr0;
	uint32_t		table_flags = 0;
	uint32_t		page_flags = 0;

	/* Create kernel page directory */
	uint32_t* kernel_dir = (uint32_t*) kernel_page_dir;
	/* Create pointer to page's tables */
	uint32_t* page_table = (uint32_t*) (kernel_page_dir + PAGE_SIZE);

	/* Clear kernel directory */
	memset(kernel_dir, 0, PAGE_SIZE);

	/* Map all memory pages for kernel */
	for (vaddr = 0; vaddr < (KERNEL_BASE + KERNEL_SIZE); vaddr += PAGE_SIZE){
		/* Translate virtual address */
		frame = vaddr >> PAGE_OFFSET_BITS;
		table_idx = frame >> PAGE_TABLE_INDEX_BITS;
		page_idx = frame & PAGE_TABLE_INDEX_MASK;

		/* Set table flags */
		table_flags = PAGE_PRESENT | PAGE_WRITEABLE | PAGE_USER;

		/* Create page table */
		*(kernel_dir + table_idx) = (uint32_t) (page_table + table_idx*0x400) | table_flags;

		/* Set page flags */
		page_flags = PAGE_PRESENT | PAGE_WRITEABLE | PAGE_USER;

		/* Create page */
		*(page_table + table_idx*0x400 + page_idx) = paddr | page_flags;

		paddr += PAGE_SIZE;
	}

	/* Load kernel directory address to CR3 */
	asm volatile ("mov %0, %%cr3"::"r"(kernel_page_dir));

	/* Switch to paging mode */
	asm volatile ("mov %%cr0, %0":"=r"(cr0));
	cr0 |= 0x80000000;
	asm volatile ("mov %0, %%cr0"::"r"(cr0));
}

/**
 * @brief Проверка карты памяти
 * 
 * @param memory_map_entry_t* mmap_addr - Карта
 * @param uint32_t length - Длина
 */
void check_memory_map(memory_map_entry_t* mmap_addr, uint32_t length){
	int i = 0;
	/* Entries number in memory map structure */
	int n = length / sizeof(memory_map_entry_t);

	/* Set pointer to memory map */
	mentry = mmap_addr;
	qemu_log("[PMM] Map:");
	for (i = 0; i < n; i++){
		qemu_log("%s [Address: %x | Length: %x]",((mentry + i)->type == 1?"Available":"Reserved"),(mentry + i)->addr,(mentry + i)->len);
		phys_memory_size += (mentry + i)->len;
	}
	qemu_log("RAM: %d MB | %d KB | %d Byte",((phys_memory_size/1024)/1024),(phys_memory_size/1024),phys_memory_size);
}

/**
 * @brief ???
 * 
 * @param physaddr_t addr - Карта
 */
void temp_map_page(physaddr_t addr){
	uint32_t table_idx = TEMP_PAGE >> 22;
	uint32_t page_idx = (TEMP_PAGE >> 12) & 0x3FF;
	uint32_t* pages = (uint32_t*) (KERNEL_PAGE_TABLE + PAGE_SIZE);

	*(pages + (table_idx << 10) + page_idx) = (uint32_t) addr | PAGE_PRESENT | PAGE_WRITEABLE;

	asm volatile ("invlpg (,%0,)"::"a"(TEMP_PAGE));
}

/**
 * @brief Получить ближайщий свободный блок
 * 
 * @param physaddr_t addr - Карта
 *
 * @return physmemory_pages_block_t* - Блок
 */
physmemory_pages_block_t* get_free_block(physaddr_t paddr){
	temp_map_page(paddr);

	return (physmemory_pages_block_t*) TEMP_PAGE;
}

/**
 * @brief ???
 * 
 * @param physaddr_t base - Карта
 * @param size_t count - Количество
 */
void free_phys_pages(physaddr_t base, size_t count){
	physmemory_pages_block_t* tmp_block;
	mutex_get(&phys_memory_mutex, true);
	/* There are no free blocks */
	if (free_phys_memory_pointer == -1){
		tmp_block = get_free_block(base);

		tmp_block->prev = base;
		tmp_block->next = base;
		tmp_block->size = count;

		free_phys_memory_pointer = base;
	} else {
		/* Get first free block address */
		physaddr_t cur_block = free_phys_memory_pointer;
		do{
			tmp_block = get_free_block(cur_block);

			/* If address after current block */
			if (base == cur_block + (tmp_block->size << PAGE_OFFSET_BITS)){
				tmp_block->size += count;

				/* If after new block is other free block*/
				if (tmp_block->next == base + (count << PAGE_OFFSET_BITS) ){
					/* Remember next block address*/
					physaddr_t next_old = tmp_block->next;

					/* Get next block*/
					tmp_block = get_free_block(next_old);

					/* Get new next block address and size */
					physaddr_t next_new = tmp_block->next;
					size_t new_count = tmp_block->size;

					/* Set current block as previous and */
					/* new next as next */
					tmp_block = get_free_block(next_new);

					tmp_block->prev = cur_block;

					tmp_block = get_free_block(cur_block);

					tmp_block->next = next_new;
					tmp_block->size += new_count;
				}

				break;
			}

			/* If address before current block */
			if (cur_block == base + (count << PAGE_OFFSET_BITS)){
				/* Remember old size and next and previous */
				size_t old_count = tmp_block->size;
				physaddr_t next = tmp_block->next;
				physaddr_t prev = tmp_block->prev;

				/* Get next block */
				tmp_block = get_free_block(next);

				/* New block is previous */
				tmp_block->prev = base;

				/* Get previous block*/
				tmp_block = get_free_block(prev);

				/* New block is next too */
				tmp_block->next = base;

				/* Get new block */
				tmp_block = get_free_block(base);

				tmp_block->next = next;
				tmp_block->prev = prev;
				tmp_block->size += old_count;

				break;
			}

			/* If address between free blocks */
			if ( cur_block > base){
				physaddr_t prev = tmp_block->next;
				tmp_block->prev = base;

				tmp_block = get_free_block(prev);
				tmp_block->next = base;

				tmp_block = get_free_block(base);
				tmp_block->next = cur_block;
				tmp_block->prev = prev;
				tmp_block->size = count;

				break;
			}

			/* or single free block */
			if (tmp_block->next == free_phys_memory_pointer){
				tmp_block->prev = base;
				physaddr_t next = tmp_block->next;

				tmp_block->next = base;

				tmp_block = get_free_block(base);

				tmp_block->prev = cur_block;
				tmp_block->next = cur_block;
				tmp_block->size = count;

				break;
			}

			cur_block = tmp_block->next;

		} while ( cur_block != free_phys_memory_pointer );

		if (base < free_phys_memory_pointer){
			free_phys_memory_pointer = base;
		}
	}
	free_pages_count += count;
	mutex_release(&phys_memory_mutex);
}

/**
 * @brief Выделить страницу в физической памяти
 * 
 * @param size_t count - Количество
 *
 * @return physaddr_t - Адрес
 */
physaddr_t alloc_phys_pages(size_t count){
	physaddr_t result = -1;
	physmemory_pages_block_t* tmp_block;
	
	qemu_log("Allocating %d physical pages", count);

	mutex_get(&phys_memory_mutex, true);	

	/* Number of free pages less than count - error!*/
	if (free_pages_count < count)
		return -1;

	if (free_phys_memory_pointer != -1){
		physaddr_t	cur_block = free_phys_memory_pointer;

		do{
			/* Get current free block*/
			tmp_block = get_free_block(cur_block);

			/* If block and count have same size*/
			if ( tmp_block->size == count ){
				/* Remember next and previous */
				physaddr_t next = tmp_block->next;
				physaddr_t prev = tmp_block->prev;

				/* Get next block */
				tmp_block = get_free_block(next);

				/* For next block - previous is previous for allocated block */
				tmp_block->prev = prev;

				/* Get previous block*/
				tmp_block = get_free_block(prev);

				/* For next block - next is next for allocated block */
				tmp_block->next = next;

				/* If it was penultimate free block */
				if (cur_block == free_phys_memory_pointer){
					/* Check next */
					free_phys_memory_pointer = next;

					/* If next block is current - we have no free memory */
					if (cur_block == free_phys_memory_pointer)
					{
						free_phys_memory_pointer = -1;
					}
				}

				/* Result address of allocated block */
				result = cur_block;
				break;
			}

			/* If current free block more than count of pages */
			if ( tmp_block->size > count ){
				/* Reduce size of current block */
				tmp_block->size -= count;

				/* Result address - current block's address + size of allocated block*/
				result = cur_block + (tmp_block->size << PAGE_OFFSET_BITS);
				break;
			}

			/* Get next free block */
			cur_block = tmp_block->next;

		} while (cur_block != free_phys_memory_pointer);

		/* If we have correct address */
		if (result != -1){
			/* Reduce free page count */
			free_pages_count -= count;
		}
	}
	
	mutex_release(&phys_memory_mutex);

	return result;
}

/**
 * @brief Инициализация менеджера памяти
 * 
 * @param uint32_t stack - Стек
 */
void init_memory_manager(uint32_t stack){
	qemu_log("Starting memory manager inittialization");
	kernel_stack = stack;

	/* Switch CPU to page memory mode*/
	switch_page_mode();
	qemu_log("Switched page memory mode");

	/* Check available memory */

	// FIXME: Not working in UEFI environmemt (hangs forever)
	memory_map_entry_t* entry = mentry;

	for (entry = mentry; entry->type; entry++){
		if ( (entry->type == 1) && (entry->addr >= 0x100000) ){
			qemu_log("Freeing %d pages on: %x", entry->len >> PAGE_OFFSET_BITS, entry->addr);
			free_phys_pages(entry->addr, entry->len >> PAGE_OFFSET_BITS);
			memory_size += entry->len;
		}
	}
	
	qemu_log("Memory size is: %d bytes.", memory_size);
	/* Map kernel heap memory */
	map_pages(KERNEL_PAGE_TABLE,
			  KERNEL_HEAP_BASE,
			  (physaddr_t) (KERNEL_MEMORY_START + KERNEL_SIZE + KERNEL_HEAP_BLOCK_INFO_SIZE),
			  KERNEL_HEAP_SIZE >> PAGE_OFFSET_BITS,
			  0x07);
	qemu_log("Mapped kernel heap memory");

	/* Map memory blocks info structure */
	map_pages(KERNEL_PAGE_TABLE,
			  (void*) (KERNEL_MEMORY_START + KERNEL_SIZE),
			  (physaddr_t) (KERNEL_MEMORY_START + KERNEL_SIZE),
			  KERNEL_HEAP_BLOCK_INFO_SIZE >> PAGE_OFFSET_BITS,
			  0x07);
	qemu_log("Mapped memory blocks info structure");

	kheap.blocks = (memory_block_t*) (KERNEL_MEMORY_START + KERNEL_SIZE);

	memset(kheap.blocks, 0, KERNEL_HEAP_BLOCK_INFO_SIZE);
	qemu_log("Zeroed heap blocks");

	kheap.count = 0;
	kheap.start = KERNEL_HEAP_BASE;
	kheap.size = KERNEL_HEAP_SIZE;
	kheap.end = kheap.start + kheap.size;
	qemu_log("Okay");
}

bool vmm_alloc_page(virtual_addr_t vaddr) {
    physaddr_t paddr = alloc_phys_pages(1);
    if (!paddr) return false;

	map_pages(KERNEL_PAGE_TABLE,
			(void*)vaddr,
			paddr,
			PAGE_SIZE,
			PAGE_WRITEABLE | PAGE_PRESENT
	);
    return true;
}

void vmm_free_page(virtual_addr_t vaddr) {
    free_phys_pages(vaddr, 1);
}

// free_phys_pages( base count )

void vmm_set_flags(physaddr_t page_dir, void* vaddr) {
	physaddr_t* tmp_page = (physaddr_t*)TEMP_PAGE;
	uint32_t page_idx = ((uint32_t) vaddr >> PAGE_OFFSET_BITS) & PAGE_TABLE_INDEX_MASK;
			
	//temp_map_page(table);
	// tmp_page[page_idx] = ()

	// TODO: ...
}

/**
 * @brief Привязывает физический адрес к виртуальному
 * 
 * @param physaddr_t page_dir - Каталог страниц
 * @param void* vaddr - Стартовый виртуальный адрес
 * @param physaddr_t paddr - Начальный физический адрес
 * @param size_t count - Размер памяти
 * @param uint32_t flags - Флаги страницы
 *
 * @return uint8_t - Возвращает 1 если нет ошибок, иначе 0
 */
uint8_t map_pages(physaddr_t page_dir, void* vaddr, physaddr_t paddr, size_t count, uint32_t flags){
	/* Pointer for access to temporary page */
	physaddr_t* tmp_page = (physaddr_t*) TEMP_PAGE;
	physaddr_t table;
	uint32_t table_flags;

	/* Create pages in cycle */
	for(; count; count--){
		uint32_t table_idx = (uint32_t) vaddr >> 22;
		uint32_t page_idx = ((uint32_t) vaddr >> PAGE_OFFSET_BITS) & PAGE_TABLE_INDEX_MASK;
		temp_map_page(page_dir);
		table = tmp_page[table_idx];
		if ( !(table & PAGE_PRESENT) ){
			physaddr_t addr = alloc_phys_pages(1);
			if (addr != -1){
				temp_map_page(addr);
				memset(tmp_page, 0, PAGE_SIZE);
				temp_map_page(page_dir);
				table_flags = PAGE_PRESENT | PAGE_WRITEABLE | PAGE_USER;
				tmp_page[table_idx] = (addr & ~PAGE_OFFSET_MASK) | table_flags;
				table = addr;
			} else {
				return FALSE;
			}
		}
		table &= ~PAGE_OFFSET_MASK;
		temp_map_page(table);
		tmp_page[page_idx] = (paddr & ~PAGE_OFFSET_MASK) | flags;
		asm volatile ("invlpg (,%0,)"::"a"(vaddr));
		vaddr += PAGE_SIZE;
		paddr += PAGE_SIZE;
	}
	return TRUE;
}

/**
 * @brief Проверка, перекрываются ли блоки
 * 
 * @param void* base1 - Блок 1
 * @param size_t size1 - Размер блока 1
 * @param void* base2 - Блок 2
 * @param size_t size2 - Размер блока 2
 *
 * @return uint8_t - ???
 */
static inline uint8_t is_blocks_overlapped(void* base1,size_t size1,void* base2,size_t size2){
	return (( base1 >= base2 ) && (base1 < base2 + size2)) ||
		   (( base2 >= base1 ) && (base2 < base1 + size1));
}

/**
 * @brief Выделение памяти
 * 
 * @param size_t size - Размер
 * @param bool align - Выравнивание
 *
 * @return void* - ???
 */
void* kmalloc_common(size_t size, bool align){
	void*	vaddr = kheap.start;
	int		i = 0;	

	mutex_t thismutex = kheap.heap_mutex;

	mutex_get(&thismutex, true);

	/* Check overlapped blocks */
	for (i = kheap.count - 1; i >= 0 ; i--){
		if (is_blocks_overlapped(kheap.blocks[i].base,kheap.blocks[i].size,vaddr,size)){
			vaddr = kheap.blocks[i].base + kheap.blocks[i].size;
		}
	}

	if (align){
		uint32_t tmp_addr = (uint32_t) vaddr;
		if (tmp_addr & 0xFFF){
			tmp_addr &= 0xFFFFF000;
			tmp_addr += PAGE_SIZE;

			vaddr = (void*) tmp_addr;
		}
	}

	for (i = kheap.count - 1; i >= 0; i--){
		kheap.blocks[i+1].base = kheap.blocks[i].base;
		kheap.blocks[i+1].size = kheap.blocks[i].size;
	}

	kheap.count++;
	kheap.blocks[0].base = vaddr;
	kheap.blocks[0].size = size;
	mutex_release(&thismutex);

	kheap.heap_mutex = thismutex;
	return vaddr;
}

/**
 * @brief Выделение памяти
 * 
 * @param size_t size - Размер
 *
 * @return void* - ???
 */
void* kmalloc(size_t size){
	return kmalloc_common(size, false);
}

/**
 * @brief Выделение памяти
 * 
 * @param size_t size - Размер
 *
 * @return void* - ???
 */
void* kmalloc_a(size_t size){
	return kmalloc_common(size, true);
}

/**
 * @brief Освобождение памяти
 * 
 * @param void* vaddr - Адрес или объект
 */
void kfree(void* vaddr){
	int		i = 0;
	int		block_idx = 0;	
	
	mutex_t thismutex = kheap.heap_mutex;

	mutex_get(&thismutex, true);

	/* Return in invalid pointer case */
	if (vaddr == NULL){
		return;
	}

	/* Find block info by virtual address */
	for (i = 0; i < kheap.count; i++){
		if (vaddr == kheap.blocks[i].base){
			block_idx = i;
			break;
		}
	}
	
	if (i == kheap.count){
		return;
	}

	/* Shift down blocks info array */
	for (i = block_idx; i < kheap.count - 1; i++){
		kheap.blocks[i].base = kheap.blocks[i+1].base;
		kheap.blocks[i].size = kheap.blocks[i+1].size;
	}
	/* Reduce number of allocated blocks */
	kheap.count--;	
	
	mutex_release(&thismutex);

	kheap.heap_mutex = thismutex;
}

/**
 * @brief Получение точки входа ядра
 * 
 * @return physaddr_t - Получить точку входа ядра
 */
physaddr_t get_kernel_dir(void){
	return kernel_page_dir;
}

/**
 * @brief Получение установленого количества ОЗУ
 * 
 * @return uint64_t - Кол-во установленной ОЗУ
 */
uint64_t getInstalledRam(){
	return phys_memory_size;
}

/**
 * @brief Выделение памяти
 */
void* kcalloc(size_t count, size_t size) {
	void* a = kmalloc(count * size);
	memset(a, 0, count*size);
	return a;
}
