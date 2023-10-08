#pragma once

#include "common.h"

typedef size_t jmp_buf[6]; // Registers: ebx, ebp, esi, edi, esp, eip

extern int setjmp(jmp_buf env);
extern void longjmp(jmp_buf env, int val);