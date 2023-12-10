#pragma once

#include <stdarg.h>
#include	"common.h"

void __com_pre_formatString(int16_t port, const char* format, va_list args);
void __com_writeString(uint16_t port, char *buf);
uint16_t __com_getInit(uint16_t key);
void __com_formatString(int16_t port, char *text, ...);
void __com_setInit(uint16_t key, uint16_t value);
int __com_init(uint16_t port);