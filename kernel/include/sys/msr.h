#pragma once

#define INTEL_THERMAL_STATUS 0x19c
#define INTEL_TEMPERATURE_TARGET 0x1a2

#define rdmsr(msr, value1, value2)  \
    __asm__ volatile("rdmsr"    \
        : "=a" (value1),            \
          "=d" (value2)             \
        : "c"  (msr)                \
    	: "edi"\
	)

#define wrmsr(msr, value1, value2)  \
    __asm__ volatile("wrmsr"    \
        :                           \
        : "c" (msr),                \
          "a" (value1),             \
          "d" (value2)              \
    )
