#include <kernel.h>

bool fpu_initialized = false;

bool fpu_isInitialized() {
	return fpu_initialized;
}

void fpu_fldcw(uint16_t cw) {
    __asm__ volatile("fldcw %0" : : "m"(cw));
}

void fpu_init() {
    uint32_t cr4 = 0;

    asm volatile ("mov %%cr4, %0":"=r"(cr4));
    cr4 |= 0x200;
    asm volatile("mov %0, %%cr4"::"r"(cr4));

    fpu_fldcw(0x37F);	

	fpu_initialized = true;
}
