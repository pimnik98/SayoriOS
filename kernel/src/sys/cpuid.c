//
// Created by ndraey on 03.10.23.
//

#include "common.h"
#include "../../include/sys/cpuid.h"
#include "../../include/sys/cpu_intel.h"
#include "io/tty.h"

size_t cpu_get_id() {
	size_t ebx = 0, unused;

	cpuid(0, unused, ebx, unused, unused);

	return ebx;
}

void cpu_get_id_string(char out[12]) {
	uint32_t* out32 = (uint32_t*)out;

	size_t ebx, ecx, edx, unused;

	cpuid(0, unused, ebx, ecx, edx);

	out32[0] = ebx;
	out32[1] = edx;
	out32[2] = ecx;
}

// Scythe-eeee-er!
struct cpu_info cpu_get_basic_info() {
	struct cpu_info info = {};

	size_t eax, ebx, extended_max, unused;

	info.manufacturer_id = cpu_get_id();

	cpuid(1, eax, ebx, info.feature_flags_ecx, info.feature_flags_edx);

	info.model_id = (eax >> 4) & 0xf;
	info.family_id = (eax >> 8) & 0xf;

	// Available only on Intel Processors.
	if(info.manufacturer_id == INTEL_MAGIC) {
		qemu_log("INTEL!");

		info.type_id = (eax >> 12) & 0x3;
		info.type_string = info.type_id < 4 ? intel_cpu_types[info.type_id] : 0;

		info.brand_id = ebx & 0xff;

		if (info.family_id == 15) {
			info.extended_family_id = (eax >> 20) & 0xff;
		}

		const char** family_models = intel_model_names[info.family_id];

		info.model_string = family_models ? family_models[info.model_id] : 0;

		cpuid(0x80000000, extended_max, unused, unused, unused);

		if(info.brand_id > 0) {
			if(eax == 0x000006B1 || eax == 0x00000F13) {
				info.brand_string = intel_additional_brand_names[info.brand_id];
			} else {
				info.brand_string = intel_brand_names[info.brand_id];
			}
		}

		size_t cache_info[4] = {0};

		if (extended_max >= 0x80000005) {
			cpuid(0x80000005,
				  unused,
				  unused,
				  cache_info[0],
				  cache_info[1]
			);
		}

		if (extended_max >= 0x80000006) {
			cpuid(0x80000006,
				  unused,
				  unused,
				  cache_info[2],
				  cache_info[3]
			);
		}

		info.l1_i_size = cache_info[0] & 0xff;
		info.l1_d_size = cache_info[1] & 0xff;
		info.l2_size = cache_info[2] & 0xffff;
		info.l3_size = cache_info[3] & 0x3fff;

		info.stepping_id = eax & 0xf;
	} else if(info.manufacturer_id == AMD_MAGIC) {
		qemu_log("AMD!");
	} else {
		qemu_log("Other!");
	}

	return info;
}