#pragma once

#define va_start(v,l)	__builtin_va_start(v,l)
#define va_end(v)	__builtin_va_end(v)
#define va_arg(v,l)	__builtin_va_arg(v,l)
#if !defined(__STRICT_ANSI__) || __STDC_VERSION__ + 0 >= 199900L || defined(__GXX_EXPERIMENTAL_CXX0X__)
#define va_copy(d,s)	__builtin_va_copy(d,s)

typedef struct {
	char *a0; /* pointer to first homed integer argument */
	int offset; /* byte offset of next parameter */
} va_list;

void va_start(va_list ap, last);
type va_arg(va_list ap, type);
void va_end(va_list ap);
