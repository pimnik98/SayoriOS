#include "common.h"
#include "sys/cpuinfo.h"
#include "sys/msr.h"
#include "sys/cpuid.h"

// X86 CPU Temperature support (partially based on memtest86+ code)

size_t cputemp_calibrate_value = 0;

/**
 * @brief Get CPU temperature sensor presence
 * @warning Works only on Intel (R) processors!
 * @return true if present, false otherwise
 */
bool is_temperature_module_present() {
	if(cpu_get_id() == INTEL_MAGIC && get_max_cpuid_count() >= 6) { // Only possible on real hardware (not QEMU)
		uint32_t info[4] = {0};

		cpuid(0x6,
			  info[0],
			  info[1],  // Not needed
			  info[2],  // Not needed
			  info[3]   // Not needed
		);

		return (bool)(info[0] & 1);
	}

	return false;
}

/**
 * @brief Get a special value
 * @warning Works only on Intel (R) processors!
 */
void cputemp_calibrate() {
	uint32_t a = 0;
	uint32_t b = 0;

	if(is_temperature_module_present()) {
		rdmsr(INTEL_TEMPERATURE_TARGET, a, b);
		cputemp_calibrate_value = (a >> 16) & 0x7F;

		// From memtest86+ code

		if (cputemp_calibrate_value < 50 || cputemp_calibrate_value > 125) {
			cputemp_calibrate_value = 100;
		}
	}
}

/**
 * @brief Get CPU temperature on x86 platforms
 * @warning Works only on Intel (R) processors!
 * @return Temperature in Celsius
 */
size_t get_cpu_temperature() {
	if(is_temperature_module_present()) {
		uint32_t a = 0;
		uint32_t b = 0;

		rdmsr(INTEL_THERMAL_STATUS, a, b);

		uint32_t absolute = (a >> 16) & 0x7F;

		return cputemp_calibrate_value - absolute;
	}

	return 0;
}
