/**
 * @file sys/gdt.c
 * @author Пиминов Никита (nikita.piminoff@yandex.ru)
 * @brief (GDT) Глобальная таблица дескрипторов
 * @version 0.3.5
 * @date 2022-10-01
 * @copyright Copyright SayoriOS Team (c) 2022-2024
 */
#include	"sys/descriptor_tables.h"
#include	"lib/string.h"

extern void gdt_flush(uint32_t);

gdt_entry_t	gdt_entries[6];
gdt_ptr_t	gdt_ptr;

extern uint32_t init_esp;
extern void tss_flush(uint32_t tr_selector);

/**
 * @brief Инициализация GDT
 */
void init_gdt(void){
	gdt_ptr.limit = (sizeof(gdt_entry_t)*6) - 1;
	gdt_ptr.base = (uint32_t)gdt_entries;
	gdt_set_gate(0, 0, 0, 0, 0);					///< Нулевой сегмент
	gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF); 	///< Сегменты кода
	gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF); 	///< Сегменты с информацией
	gdt_set_gate(3, 0, 0xFFFFFFFF, 0xFA, 0xCF);		///< Пользовательские сегменты
	gdt_set_gate(4, 0, 0xFFFFFFFF, 0xF2, 0xCF);		///< Пользовательские сегменты
	write_tss(5, 0x10, init_esp);

	gdt_flush( (uint32_t) &gdt_ptr);
	tss_flush(0x28);
}

/**
 * @brief Инициализация GDT и IDT
 */
void init_descriptor_tables(void){
	init_gdt();
	init_idt();
}

/**
 * @brief Установка сегмента
 *
 * @param num - ???
 * @param base - ???
 * @param limit - ???
 * @param access - ???
 * @param gran - ???
 */
void gdt_set_gate(int32_t num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran){
	gdt_entries[num].base_low = (base & 0xFFFF);
	gdt_entries[num].base_middle = (base >> 16) & 0xFF;
	gdt_entries[num].base_high = (base >> 24) & 0xFF;
	
	gdt_entries[num].limit_low = (limit & 0xFFFF);
	gdt_entries[num].granularity = (limit >> 16) & 0xF;
	
	gdt_entries[num].granularity |= gran & 0xF0;
	gdt_entries[num].access = access;
}