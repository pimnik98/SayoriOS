#pragma once

#include "sys/cpuid.h"

int do_intel(bool silent);
int do_amd(bool silent);
char* printregs(int eax, int ebx, int ecx, int edx);
int detect_cpu(bool silent);
char* getNameBrand();
size_t get_max_cpuid_count();
bool is_temperature_module_present();
size_t get_cpu_temperature();