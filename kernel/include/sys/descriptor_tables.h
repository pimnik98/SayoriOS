#ifndef       DESCRIPTOR_TABLES_H
#define       DESCRIPTOR_TABLES_H

#include  "common.h"

struct gdt_entry_struct
{
    uint16_t  limit_low;
    uint16_t  base_low;
    uint8_t   base_middle;
    uint8_t   access;
    uint8_t   granularity;
    uint8_t   base_high;

}__attribute__((packed));

typedef struct gdt_entry_struct gdt_entry_t;

struct gdt_ptr_struct
{
  uint16_t    limit;
  uint32_t    base;
}__attribute__((packed));

typedef struct gdt_ptr_struct gdt_ptr_t;

struct    tss_entry
{
    uint32_t  prev_tss;
    uint32_t  esp0;          /* Указатель текущего стека ядра */
    uint32_t  ss0;           /* Селектор сегмента текущего стека ядра */
    uint32_t  esp1;
    uint32_t  ss1;
    uint32_t  esp2;
    uint32_t  ss2;
    uint32_t  cr3;
    uint32_t  eip;
    uint32_t  eflags;
    uint32_t  eax;
    uint32_t  ecx;
    uint32_t  edx;
    uint32_t  ebx;
    uint32_t  esp;
    uint32_t  ebp;
    uint32_t  esi;
    uint32_t  edi;
    uint32_t  es;
    uint32_t  cs;
    uint32_t  ss;
    uint32_t  ds;
    uint32_t  fs;
    uint32_t  gs;
    uint32_t  ldtr;
    uint16_t  task_flags;
    uint16_t  iomap_offset;  /* Смещение от начала TSS до I/O map */
    uint8_t   iomap;         /* Байт-терминатор */
} __attribute__((packed));

typedef struct tss_entry tss_entry_t;

struct tss_descriptor
{
    uint16_t  limit_15_0;       /* Биты 15-0 лимита */
    uint16_t  base_15_0;        /* Биты 15-0 базы */
    uint8_t   base_23_16;       /* Биты 23-16 базы */
    uint8_t   type:4;           /* Тип сегмента */
    uint8_t   sys:1;            /* Системный сегмент */
    uint8_t   DPL:2;            /* Уровень привилегий сегмента */
    uint8_t   present:1;        /* Бит присутствия */
    uint8_t   limit_19_16:4;    /* Биты 19-16 лимита */
    uint8_t   AVL:1;            /* Зарезервирован */
    uint8_t   allways_zero:2;   /* Всегда нулевые */
    uint8_t   gran:1;           /* Бит гранулярности */
    uint8_t   base_31_24;       /* Биты 31-24 базы */
}__attribute__((packed));

typedef struct tss_descriptor tss_descriptor_t;

struct idt_entry_struct
{
  uint16_t    base_low;
  uint16_t    selector;
  uint8_t     allways0;
  uint8_t     flags;
  uint16_t    base_high;
}__attribute__((packed));

typedef struct idt_entry_struct idt_entry_t;

struct    idt_ptr_struct
{
  uint16_t    limit;
  uint32_t    base;
}__attribute__((packed));

typedef   struct  idt_ptr_struct idt_ptr_t;

/* External function for interrupt processing */
extern void isr0(void);
extern void isr1(void);
extern void isr2(void);
extern void isr3(void);
extern void isr4(void);
extern void isr5(void);
extern void isr6(void);
extern void isr7(void);

extern void isr8(void);
extern void isr9(void);
extern void isr10(void);
extern void isr11(void);
extern void isr12(void);
extern void isr13(void);
extern void isr14(void);
extern void isr15(void);

extern void isr16(void);
extern void isr17(void);
extern void isr18(void);
extern void isr19(void);
extern void isr20(void);
extern void isr21(void);
extern void isr22(void);
extern void isr23(void);

extern void isr24(void);
extern void isr25(void);
extern void isr26(void);
extern void isr27(void);
extern void isr28(void);
extern void isr29(void);
extern void isr30(void);
extern void isr31(void);

extern void irq0(void);
extern void irq1(void);
extern void irq2(void);
extern void irq3(void);
extern void irq4(void);
extern void irq5(void);
extern void irq6(void);
extern void irq7(void);
extern void irq8(void);
extern void irq9(void);
extern void irq10(void);
extern void irq11(void);
extern void irq12(void);
extern void irq13(void);
extern void irq14(void);
extern void irq15(void);

extern void isr80(void);

#endif
