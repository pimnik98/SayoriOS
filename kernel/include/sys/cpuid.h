//
// Created by ndraey on 03.10.23.
//

#pragma once

#define cpuid(in, a, b, c, d)    \
	__asm__ __volatile__("cpuid" \
	               : "=a" (a),   \
					 "=b" (b),   \
					 "=c" (c),   \
					 "=d" (d)    \
				   : "a" (in)    \
    )

#define cpuid_count(in, count, a, b, c, d)    \
	__asm__ __volatile__("cpuid" \
	               : "=a" (a),   \
					 "=b" (b),   \
					 "=c" (c),   \
					 "=d" (d)    \
				   : "0" (in),    \
				     "2" (count)    \
    )

#define INTEL_MAGIC 0x756e6547
#define AMD_MAGIC   0x68747541


static const char* cpu_flag_edx_description[] = {
		"fpu",
		"vme",
		"debugging",
		"pse",
		"rdtsc",
		"msr",
		"pae",
		"mce",
		"cx8",
		"apic",
		"'reserved 10'",
		"'sysenter/sysexit'",
		"mtrr",
		"pge",
		"mca",
		"cmov",
		"pat",
		"pse-36",
		"psn",
		"clfsh",
		"nx",
		"ds",
		"acpi",
		"mmx",
		"fxsr",
		"sse",
		"sse2",
		"ss",
		"htt",
		"tm",
		"ia64",
		"pbe"
};

struct cpu_info {
	size_t manufacturer_id;
	size_t model_id;
	size_t family_id;
	size_t extended_family_id;
	size_t type_id;
	size_t brand_id;
	size_t stepping_id;

	const char* brand_string;
	const char* model_string;
	const char* type_string;

	size_t l1_i_size;
	size_t l1_d_size;
	size_t l2_size;
	size_t l3_size;

	size_t feature_flags_ecx;
	size_t feature_flags_edx;
};

size_t cpu_get_id();
struct cpu_info cpu_get_basic_info();