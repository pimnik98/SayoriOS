#include "common.h"
#include "sys/cpuid.h"
#include "io/ports.h"
#include "sys/msr.h"
#include "sys/mtrr.h"
#include "io/tty.h"

#define UNCACHED 0
#define WRITECOMBINING 1
#define WRITETHROUGH 4
#define WRITEPROTECT 5
#define WRITEBACK 6

bool mtrr_is_supported = false;
bool mtrr_wc_available = false;
bool mtrr_fixed_size_available = false;
size_t variable_size_mtrrs = 0;

#define CALC_MASK(size) (~(size - 1) & 0xffffffff)

void mtrr_init() {
	uint32_t unused, eax, edx;

	cpuid(1, unused, unused, unused, edx);

	qemu_log("%x", edx & (1 << 12));

	if(edx & (1 << 12)) {
		mtrr_is_supported = true;
	} else {
		return;
	}

	rdmsr(0xFE, eax, unused);

	variable_size_mtrrs = eax & 0xff;
	mtrr_fixed_size_available = eax & (1 << 8) ? true : false;
	mtrr_wc_available = eax & (1 << 10) ? true : false;

	qemu_log("Variable size MTRRs count: %d", variable_size_mtrrs);
	qemu_log("Fixed size available: %d", mtrr_fixed_size_available);
	qemu_log("Writecombining available: %d", mtrr_wc_available);

	for(size_t i = 0; i < variable_size_mtrrs; i++) {
		uint32_t base, mask;

		read_mtrr(i, &base, &mask);

		qemu_log("[%d] MTRR: base: %x; mask: %x", i, base, mask);
	}
}

void list_mtrrs() {
	tty_printf("Physical FB is: %x", getDisplayAddr());

	for(size_t i = 0; i < variable_size_mtrrs; i++) {
		uint32_t base, mask;

		read_mtrr(i, &base, &mask);

		qemu_log("[%d] MTRR: base: %x; mask: %x", i, base, mask);
		tty_printf("[%d] MTRR: base: %x; mask: %x\n", i, base, mask);
	}
}

uint32_t get_mtrr_index(uint32_t address) {
    for(size_t i = 0; i < variable_size_mtrrs; i++) {
        uint32_t base, mask;

        read_mtrr(i, &base, &mask);

        if((mask & (1 << 11)) && (base & ~0xfff) == ALIGN(address, 4096)) {
            return i;
        }
    }

    return 0xffffffff;
}


void read_mtrr(size_t index, uint32_t* base, uint32_t* mask) {
	uint32_t unused;

	if(index >= variable_size_mtrrs)
		return;

	rdmsr(0x200 + (index * 2), *base, unused);
	rdmsr(0x201 + (index * 2), *mask, unused);
}

void write_mtrr(size_t index, uint32_t base, uint32_t mask) {
	uint32_t unused = 0;

	if(index >= variable_size_mtrrs)
		return;

	wrmsr(0x200 + (index * 2), base, unused);
	wrmsr(0x201 + (index * 2), mask, unused);
}

void write_mtrr_size(size_t index, uint32_t base, uint32_t size, size_t type) {
	uint32_t unused = 0;

	if(index >= variable_size_mtrrs)
		return;

	wrmsr(0x200 + (index * 2), ALIGN(base, 4096) | type, unused);
	wrmsr(0x201 + (index * 2), CALC_MASK(ALIGN(size, 4096)) | (1 << 11), unused);
}

size_t find_free_mtrr() {
	for(size_t i = 0; i < variable_size_mtrrs; i++) {
		uint32_t base, mask;

		read_mtrr(i, &base, &mask);

		if((mask & (1 << 11)) == 0) {
			return i;
		}
	}

	return 0xffffffff;  // Not found
}
