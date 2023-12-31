/**
 * @file fpu.c
 * @author NDRAEY >_ (pikachu_andrey@vk.com)
 * @brief FPU
 * @version 0.3.4
 * @date 2023-12-19
 * @copyright Copyright SayoriOS Team (c) 2022-2023
 */
#include "common.h"
#include "io/ports.h"

bool fpu_initialized = false;

/// Возвращает статус FPU
bool fpu_isInitialized() {
	return fpu_initialized;
}

/// Для инициализации FPU
void fpu_fldcw(const uint16_t cw) {
    __asm__ volatile("fldcw %0" : : "m"(cw));
}

/// Инициализация FPU
void fpu_init() {
	size_t cr4 = 0;

	__asm__ volatile ("mov %%cr4, %0":"=r"(cr4));
	cr4 |= 0x200;
	__asm__ volatile("mov %0, %%cr4"::"r"(cr4));

	fpu_fldcw(0x37F);

	fpu_initialized = true;

	qemu_log("FPU init result: %f (should be 0.5)", 1.0 / 2.0);
}
