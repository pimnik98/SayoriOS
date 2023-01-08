#ifndef			CPU_ISR_H
#define			CPU_ISR_H

#define			INT_0		0x00
#define			INT_1		0x01
#define			INT_2		0x02
#define			INT_3		0x03
#define			INT_4		0x04
#define			INT_5		0x05
#define			INT_6		0x06
#define			INT_7		0x07
#define			INT_8		0x08
#define			INT_9		0x09
#define			INT_10		0x0A
#define			INT_11		0x0B
#define			INT_12		0x0C
#define			INT_13		0x0D
#define			INT_14		0x0E
#define			INT_15		0x0F
#define			INT_16		0x10
#define			INT_17		0x11
#define			INT_18		0x12

#define			EXT_BIT			(1 << 0)
#define			IDT_BIT			(1 << 1)
#define			TI_BIT			(1 << 2)
#define			ERR_CODE_MASK	0xFFF8;

#include		"common.h"
#include		"sys/isr.h"

/*------------------------------------------------------------------------------
//		Handlers prototypes
//----------------------------------------------------------------------------*/

/* INT 00h - division by zero */
void division_by_zero(registers_t regs);

/* INT 06h - fault opcode */
void fault_opcode(registers_t regs);

/* INT 08h - double error */
void double_error(registers_t regs);

/* INT 0Ah - invalid TSS */
void invalid_tss(registers_t regs);

/* INT 0Bh - Segment is't available */
void segment_is_not_available(registers_t regs);

/* INT 0Ch - Stack error */
void stack_error(registers_t regs);

/* INT 0Dh - General protection error */
void general_protection_error(registers_t regs);

/* INT 0Eh - page fault */
void page_fault(registers_t regs);

uint32_t extern read_cr2();

#endif
