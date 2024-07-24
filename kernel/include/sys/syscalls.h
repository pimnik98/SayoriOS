#pragma once

#include	"common.h"
#include	"sys/isr.h"

#define		SYSCALL					0x50

typedef size_t syscall_fn_t (size_t, size_t, size_t);

#define		NUM_CALLS	19

void init_syscalls(void);
extern size_t syscall_entry_call(void* entry_point, void* param1, void* param2, void* param3);
void syscall_handler(registers_t regs);
