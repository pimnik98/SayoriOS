/**
 * @file sys/gdt.c
 * @author Пиминов Никита (nikita.piminoff@yandex.ru)
 * @brief (GDT) Глобальная таблица дескрипторов
 * @version 0.3.3
 * @date 2022-10-01
 * @copyright Copyright SayoriOS Team (c) 2022-2023
 */
#include	"sys/descriptor_tables.h"
#include	"lib/string.h"
#include	"io/ports.h"

tss_entry_t	tss;

extern gdt_entry_t	gdt_entries[6];

extern uint32_t init_esp;
extern void tss_flush(uint32_t tr_selector);

/**
 * @brief ???
 *
 * @param int32_t num - ???
 * @param uint32_t ss0 - ???
 * @param uint32_t esp0 - ???
 */
void write_tss(int32_t num, uint32_t ss0, uint32_t esp0){
	memset(&tss, 0, sizeof(tss_entry_t));
	tss.ss0 = ss0;
	tss.esp0 = esp0;
	tss.cs = 0x08;
	tss.ss = tss.ds = tss.es = tss.fs = tss.gs = 0x10;
	tss.iomap = 0xFF;
	tss.iomap_offset = (uint16_t) ( (uint32_t) &tss.iomap - (uint32_t) &tss );
	uint32_t base = (uint32_t) &tss;
	uint32_t limit = sizeof(tss)-1;
	tss_descriptor_t* tss_d = (tss_descriptor_t*) &gdt_entries[num];
	tss_d->base_15_0 = base & 0xFFFF;
	tss_d->base_23_16 = (base >> 16) & 0xFF;
	tss_d->base_31_24 = (base >> 24) & 0xFF;
	tss_d->limit_15_0 = limit & 0xFFFF;
	tss_d->limit_19_16 = (limit >> 16) & 0xF;
	tss_d->present = 1;
	tss_d->sys = 0;
	tss_d->DPL = 0;
	tss_d->type = 9;
	tss_d->AVL = 0;
	tss_d->allways_zero = 0;
	tss_d->gran = 0;
}

/**
 * @brief ???
 *
 * @param uint32_t stack - ???
 */
void set_kernel_stack_in_tss(uint32_t stack) {
	tss.esp0 = stack;
}

/**
 * @brief ???
 *
 * @return uint32_t - ???
 */
uint32_t get_tss_esp0(){
	return tss.esp0;
}
