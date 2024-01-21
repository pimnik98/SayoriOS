/**
 * @file sys/gdt.c
 * @author Пиминов Никита (nikita.piminoff@yandex.ru)
 * @brief (GDT) Глобальная таблица дескрипторов
 * @version 0.3.5
 * @date 2022-10-01
 * @copyright Copyright SayoriOS Team (c) 2022-2024
 */
#include  "sys/descriptor_tables.h"
#include  "lib/string.h"
#include  "io/ports.h"

extern void gdt_flush(uint32_t);
extern void tss_flush(uint32_t tr_selector);
extern uint32_t init_esp;

tss_entry_t tss;
gdt_entry_t gdt_entries[6];
gdt_ptr_t   gdt_ptr;

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
    /* Извлекаем нижнюю часть базы адреса (биты 0-15) */
    gdt_entries[num].base_low = (base & 0xFFFF);
    /* Извлекаем среднюю часть базы адреса (биты 16-23) */
    gdt_entries[num].base_middle = (base >> 16) & 0xFF;
    /* Извлекаем верхнюю часть базы (биты 24-31) */
    gdt_entries[num].base_high = (base >> 24) & 0xFF;
    /* Извлекаем нижнюю часть лимита (биты 0-15) */
    gdt_entries[num].limit_low = (limit & 0xFFFF);
    /* Granulary - это байт, который мы получаем,
       сдвинув limit на два байта вправо, при этом
       верхний полубайт будет содержать флаги (мы
       учитываем это, когда формируем передавемый
       в параметрах limit). На первом этапе мы
       получам верхнюю часть limit */
    gdt_entries[num].granularity = (limit >> 16) & 0xF;
    /* Тепер полученные биты объединяем с верхним
       полубайтом gran. С текущими передаваемыми
       параметрами, мы приходим к тому, что
       устанавливаются флаги G и D/B, что дает нам
       размер страницы в четырехкилобайтовых единицах
       и 32-двух разрядные смещения при доступе к ней */
    gdt_entries[num].granularity |= gran & 0xF0;
    /* Access переписываем без изменений */
    gdt_entries[num].access = access;
}


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

void set_kernel_stack_in_tss(uint32_t stack) {
    tss.esp0 = stack;
}

uint32_t get_tss_esp0(){
    return tss.esp0;
}

#define GDT_NUMBER_OF_ELTS 6

/**
 * @brief Инициализация GDT
 */
void init_gdt(void){
    /* Устанавливается размер GDT в байтах. */
    gdt_ptr.limit = ( sizeof(gdt_entry_t) * GDT_NUMBER_OF_ELTS ) - 1;
    /* Устанавливается базовый адрес GDT */
    gdt_ptr.base = (uint32_t)gdt_entries;
    /* Устанавливается нулевой дескриптор в GDT.
       p:0 dpl:0 s:0(sys)
       type:0(Запрещенное значение) */
    gdt_set_gate(0, 0, 0, 0, 0);
    /* p:1 dpl:3 s:1(user)
       type:1010(Сегмент кода для выполнения/чтения)
       Нулевой сегмент */
    gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);
    /* p:1 dpl:0 s:1(user)
       type:0010(Сегмент данных для чтения/записи)
       Сегменты с информацией */
    gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF);
    /* p:1 dpl:3 s:1(user)
       type:1010(Сегмент кода для выполнения/чтения)
       Пользовательские сегменты */
    gdt_set_gate(3, 0, 0xFFFFFFFF, 0xFA, 0xCF);
    /* p:1 dpl:3 s:1(user)
       type:1010(Сегмент кода для выполнения/чтения)
       Пользовательские сегменты */
    gdt_set_gate(4, 0, 0xFFFFFFFF, 0xF2, 0xCF);

    write_tss(5, 0x10, init_esp);

    gdt_flush( (uint32_t) &gdt_ptr);
    tss_flush(0x28);
}

/**
 * @file sys/gdt.c
 * @author Пиминов Никита (nikita.piminoff@yandex.ru)
 * @brief (GDT) Глобальная таблица дескрипторов
 * @version 0.3.5
 * @date 2022-10-01
 * @copyright Copyright SayoriOS Team (c) 2022-2024
 */
#include  "sys/descriptor_tables.h"
#include  "lib/string.h"
#include  "io/ports.h"

extern gdt_entry_t  gdt_entries[6];
idt_entry_t         idt_entries[256];
idt_ptr_t           idt_ptr;

extern uint32_t init_esp;
extern void idt_flush(uint32_t);

void idt_set_gate(uint8_t num, uint32_t base, uint16_t selector, uint8_t flags){
    idt_entries[num].base_low = base & 0xFFFF;
    idt_entries[num].base_high = (base >> 16) & 0xFFFF;
    idt_entries[num].selector = selector;
    idt_entries[num].allways0 = 0;
    idt_entries[num].flags = flags; /* - для пользовательского режима */
}

void init_idt(void) {
    idt_ptr.limit = sizeof(idt_entry_t)*256 - 1;
    idt_ptr.base = (uint32_t)idt_entries;
    memset(idt_entries, 0, sizeof(idt_entry_t)*256);
    outb(0x20, 0x11);
    outb(0xA0, 0x11);
    outb(0x21, 0x20);
    outb(0xA1, 0x28);
    outb(0x21, 0x04);
    outb(0xA1, 0x02);
    outb(0x21, 0x01);
    outb(0xA1, 0x01);
    outb(0x21, 0x0);
    outb(0xA1, 0x0);

    idt_set_gate(0, (uint32_t)isr0, 0x08, 0x8E);
    idt_set_gate(1, (uint32_t)isr1, 0x08, 0x8E);
    idt_set_gate(2, (uint32_t)isr2, 0x08, 0x8E);
    idt_set_gate(3, (uint32_t)isr3, 0x08, 0x8E);
    idt_set_gate(4, (uint32_t)isr4, 0x08, 0x8E);
    idt_set_gate(5, (uint32_t)isr5, 0x08, 0x8E);
    idt_set_gate(6, (uint32_t)isr6, 0x08, 0x8E);
    idt_set_gate(7, (uint32_t)isr7, 0x08, 0x8E);

    idt_set_gate(8, (uint32_t)isr8, 0x08, 0x8E);
    idt_set_gate(9, (uint32_t)isr9, 0x08, 0x8E);
    idt_set_gate(10, (uint32_t)isr10, 0x08, 0x8E);
    idt_set_gate(11, (uint32_t)isr11, 0x08, 0x8E);
    idt_set_gate(12, (uint32_t)isr12, 0x08, 0x8E);
    idt_set_gate(13, (uint32_t)isr13, 0x08, 0x8E);
    idt_set_gate(14, (uint32_t)isr14, 0x08, 0x8E);
    idt_set_gate(15, (uint32_t)isr15, 0x08, 0x8E);

    idt_set_gate(16, (uint32_t)isr16, 0x08, 0x8E);
    idt_set_gate(17, (uint32_t)isr17, 0x08, 0x8E);
    idt_set_gate(18, (uint32_t)isr18, 0x08, 0x8E);
    idt_set_gate(19, (uint32_t)isr19, 0x08, 0x8E);
    idt_set_gate(20, (uint32_t)isr20, 0x08, 0x8E);
    idt_set_gate(21, (uint32_t)isr21, 0x08, 0x8E);
    idt_set_gate(22, (uint32_t)isr22, 0x08, 0x8E);
    idt_set_gate(23, (uint32_t)isr23, 0x08, 0x8E);

    idt_set_gate(24, (uint32_t)isr24, 0x08, 0x8E);
    idt_set_gate(25, (uint32_t)isr25, 0x08, 0x8E);
    idt_set_gate(26, (uint32_t)isr26, 0x08, 0x8E);
    idt_set_gate(27, (uint32_t)isr27, 0x08, 0x8E);
    idt_set_gate(28, (uint32_t)isr28, 0x08, 0x8E);
    idt_set_gate(29, (uint32_t)isr29, 0x08, 0x8E);
    idt_set_gate(30, (uint32_t)isr30, 0x08, 0x8E);
    idt_set_gate(31, (uint32_t)isr31, 0x08, 0x8E);

    idt_set_gate(32, (uint32_t)irq0, 0x08, 0x8E);
    idt_set_gate(33, (uint32_t)irq1, 0x08, 0x8E);
    idt_set_gate(34, (uint32_t)irq2, 0x08, 0x8E);
    idt_set_gate(35, (uint32_t)irq3, 0x08, 0x8E);
    idt_set_gate(36, (uint32_t)irq4, 0x08, 0x8E);
    idt_set_gate(37, (uint32_t)irq5, 0x08, 0x8E);
    idt_set_gate(38, (uint32_t)irq6, 0x08, 0x8E);
    idt_set_gate(39, (uint32_t)irq7, 0x08, 0x8E);
    idt_set_gate(40, (uint32_t)irq8, 0x08, 0x8E);
    idt_set_gate(41, (uint32_t)irq9, 0x08, 0x8E);
    idt_set_gate(42, (uint32_t)irq10, 0x08, 0x8E);
    idt_set_gate(43, (uint32_t)irq11, 0x08, 0x8E);
    idt_set_gate(44, (uint32_t)irq12, 0x08, 0x8E);
    idt_set_gate(45, (uint32_t)irq13, 0x08, 0x8E);
    idt_set_gate(46, (uint32_t)irq14, 0x08, 0x8E);
    idt_set_gate(47, (uint32_t)irq15, 0x08, 0x8E);

    /* System calls */
    idt_set_gate(0x50, (uint32_t)isr80, 0x08, 0xEF);

    idt_flush((uint32_t) &idt_ptr);
}


void init_descriptor_tables(void){
    init_gdt();
    init_idt();
}
