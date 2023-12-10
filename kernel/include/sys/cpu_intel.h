//
// Created by ndraey on 03.10.23.
//

#pragma once

static const char *intel_brand_names[] = {
	"Brand ID Not Supported.",
	"Intel(R) Celeron(R) processor",
	"Intel(R) Pentium(R) III processor",
	"Intel(R) Pentium(R) III Xeon(R) processor",
	"Intel(R) Pentium(R) III processor",
	"Reserved",
	"Mobile Intel(R) Pentium(R) III processor-M",
	"Mobile Intel(R) Celeron(R) processor",
	"Intel(R) Pentium(R) 4 processor",
	"Intel(R) Pentium(R) 4 processor",
	"Intel(R) Celeron(R) processor",
	"Intel(R) Xeon(R) Processor",
	"Intel(R) Xeon(R) processor MP",
	"Reserved",
	"Mobile Intel(R) Pentium(R) 4 processor-M",
	"Mobile Intel(R) Pentium(R) Celeron(R) processor",
	"Reserved",
	"Mobile Genuine Intel(R) processor",
	"Intel(R) Celeron(R) M processor",
	"Mobile Intel(R) Celeron(R) processor",
	"Intel(R) Celeron(R) processor",
	"Mobile Geniune Intel(R) processor",
	"Intel(R) Pentium(R) M processor",
	"Mobile Intel(R) Celeron(R) processor"
};

static const char *intel_additional_brand_names[] = {
	"Reserved",
	"Reserved",
	"Reserved",
	"Intel(R) Celeron(R) processor",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Intel(R) Xeon(R) processor MP",
	"Reserved",
	"Reserved",
	"Intel(R) Xeon(R) processor",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved"
};

static const char* intel_cpu_types[] = {
	"Original OEM",
	"Overdrive",
	"Dual-capable",
	"Reserved"
};

static const char* intel_cpu_family_names[] = {
	[3] = "i386",
	[4] = "i486",
	[5] = "Pentium",
	[6] = "Pentium Pro",
	[15] = "Pentium 4"
};

static const char* intel_model_names[][9] = {
		[0] = {},
		[1] = {},
		[2] = {},
		[3] = {},
		[4] = {
			[0] = "DX",
			[1] = "DX",
			[2] = "SX",
			[3] = "487/DX2",
			[4] = "SL",
			[5] = "SX2",
			[7] = "Write-back enchanced SX2",
			[8] = "DX4",
		},
		[5] = {
			[1] = "60/66",
			[2] = "75-200",
			[3] = "For 486 systems",
			[4] = "MMX",
		},
		[6] = {
			[1] = "Pentium Pro",
			[3] = "Pentium II Model 3",
			[5] = "Pentium II Model 5 / Xeon / Celeron",
			[6] = "Celeron",
			[7] = "Pentium III / Pentium III Xeon - external L2 cache",
			[8] = "Pentium III / Pentium III Xeon - internal L2 cache",
		}
};