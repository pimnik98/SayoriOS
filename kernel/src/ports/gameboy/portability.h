#pragma once

#include "sys/memory.h"

#define malloc(a) kmalloc(a)
#define free(a) kfree(a)
#define printf(a, ...) qemu_printf(a, ##__VA_ARGS__)
