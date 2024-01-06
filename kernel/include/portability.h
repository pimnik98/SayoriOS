#pragma once

#include "mem/vmm.h"
#include "lib/stdlib.h"

#define malloc(a) kmalloc(a)
#define calloc(a, b) kcalloc(a, b)
#define realloc(a, b) krealloc(a, b)
#define free(a) kfree(a)
#define printf(a, ...) qemu_printf(a, ##__VA_ARGS__)


#define passert(expression) \
    if (!(expression)) { \
        qemu_warn("Assertion failed: file %s, line %d\n", __FILE__, __LINE__); \
    }

// stdint.h

#define INT_FAST16_MIN	(-2147483647-1)
#define INT_FAST32_MIN	(-2147483647-1)
#define INT_FAST16_MAX	(2147483647)
#define INT_FAST32_MAX	(2147483647)
#define UINT_FAST16_MAX	(4294967295U)
#define UINT_FAST32_MAX	(4294967295U)
#define SIZE_MAX		(4294967295U)

#define INT32_MIN	(-INT32_MAX-1)
#define INT32_MAX	2147483647

#define INT_MIN	(-INT_MAX - 1)
#define INT_MAX	2147483647

#define UINT_MAX	4294967295U

#define UINT32_MAX		(4294967295U)


// bits/types.h

typedef signed char __int8_t;
typedef unsigned char __uint8_t;
typedef signed short int __int16_t;
typedef unsigned short int __uint16_t;
typedef signed int __int32_t;
typedef unsigned int __uint32_t;
typedef unsigned long long __uint64_t;
typedef long long __int64_t;

typedef __int8_t __int_least8_t;
typedef __uint8_t __uint_least8_t;
typedef __int16_t __int_least16_t;
typedef __uint16_t __uint_least16_t;
typedef __int32_t __int_least32_t;
typedef __uint32_t __uint_least32_t;
typedef __int64_t __int_least64_t;
typedef __uint64_t __uint_least64_t;

// stdint.h
typedef __int_least8_t int_least8_t;
typedef __int_least16_t int_least16_t;
typedef __int_least32_t int_least32_t;
typedef __int_least64_t int_least64_t;


typedef __uint_least8_t uint_least8_t;
typedef __uint_least16_t uint_least16_t;
typedef __uint_least32_t uint_least32_t;
typedef __uint_least64_t uint_least64_t;

typedef signed char		int_fast8_t;
typedef int			int_fast16_t;
typedef int			int_fast32_t;

typedef uint8_t		uint_fast8_t;
typedef uint32_t	uint_fast16_t;
typedef uint32_t	uint_fast32_t;

typedef int			intptr_t;
typedef unsigned int		uintptr_t;

__extension__ typedef long long int __intmax_t;
__extension__ typedef unsigned long long int __uintmax_t;

typedef __intmax_t		intmax_t;
typedef __uintmax_t		uintmax_t;


typedef __uint_least64_t uint_least64_t;
typedef __int_least64_t int_least64_t;

typedef long long int	int_fast64_t;
typedef unsigned long long int	uint_fast64_t;

// bits/types/struct_tm.h

struct tm {
  int tm_sec;			/* Seconds.	[0-60] (1 leap second) */
  int tm_min;			/* Minutes.	[0-59] */
  int tm_hour;			/* Hours.	[0-23] */
  int tm_mday;			/* Day.		[1-31] */
  int tm_mon;			/* Month.	[0-11] */
  int tm_year;			/* Year	- 1900.  */
  int tm_wday;			/* Day of week.	[0-6] */
  int tm_yday;			/* Days in year.[0-365]	*/
  int tm_isdst;			/* DST.		[-1/0/1]*/

  long int __tm_gmtoff;		/* Seconds east of UTC.  */
  const char *__tm_zone;	/* Timezone abbreviation.  */
};

typedef int32_t time_t;
