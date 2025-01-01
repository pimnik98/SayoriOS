/**
 * @file sys/isr.c
 * @author Пиминов Никита (nikita.piminoff@yandex.ru)
 * @brief Обработчик прерывания высокого уровня
 * @version 0.3.5
 * @date 2022-10-01
 * @copyright Copyright SayoriOS Team (c) 2022-2024
 */

#include "sys/isr.h"
#include "io/ports.h"
#include "sys/cpu_isr.h"

volatile isr_t	interrupt_handlers[256];

void isr_handler(const registers_t regs){
    if (interrupt_handlers[regs.int_num] != 0){
        isr_t handler = interrupt_handlers[regs.int_num];
        handler(regs);
    }
}

void irq_handler(registers_t regs){
    if (regs.int_num >= 40){
        outb(0xA0, 0x20);
    }

    outb(0x20, 0x20);

    if (interrupt_handlers[regs.int_num] != 0){
        isr_t handler = interrupt_handlers[regs.int_num];
        handler(regs);
    }
}

/* @param n - Номер обработчика */
/* @param handler - Функция обработчик */
void register_interrupt_handler(uint8_t n, isr_t handler){
    qemu_warn("Updated handler for IRQ%d", n);

    interrupt_handlers[n] = handler;
}

/* Инициализация ISR */
void isr_init(){
    register_interrupt_handler(INT_0, &division_by_zero);
    register_interrupt_handler(INT_6, &fault_opcode);
    register_interrupt_handler(INT_8, &double_error);
    register_interrupt_handler(INT_10, &invalid_tss);
    register_interrupt_handler(INT_11, &segment_is_not_available);
    register_interrupt_handler(INT_12, &stack_error);
    register_interrupt_handler(INT_13, &general_protection_error);
    register_interrupt_handler(INT_14, &page_fault);
    register_interrupt_handler(INT_16, &fpu_fault);
}
