/**
 * @file common.h
 * @author Пиминов Никита (nikita.piminoff@yandex.ru)
 * @brief Основные определения ядра
 * @version 0.3.5
 * @date 2023-12-07
 * @copyright Copyright SayoriOS Team (c) 2022-2024
 */
#pragma once

#define		FALSE			0
#define		TRUE			1

#ifndef __cplusplus

typedef enum {
    false = 0,
    true = 1
} bool;

#define nullptr ((void*)0)
#define NULL (0)

#endif

#define SAYORI_INLINE static inline __attribute__((always_inline))

#define KB (1 << 10)
#define MB (1 << 20)
#define GB (1 << 30)

#define ALIGN(value, align) ((value) + ((-(value)) & ((align) - 1)))
#define IS_ALIGNED(value, align) ((value) % (align) == 0)

/* 64-bit types */
typedef	unsigned long long	uint64_t;
typedef	long long			int64_t;
/* 32-bit types */
typedef	unsigned int	uint32_t;
typedef	int		int32_t;
/* 16-bit types */
typedef	unsigned short	uint16_t;
typedef	short		int16_t;
/* 8-bit types */
typedef	unsigned char	uint8_t;
typedef	char		int8_t;

#ifdef SAYORI64
typedef	uint64_t		size_t;
typedef	int64_t			ssize_t;
#else
typedef	uint32_t		size_t;
typedef	int32_t			ssize_t;
#endif




struct registers {
    uint32_t	ds;
    uint32_t	edi, esi, ebp, esp, ebx, edx, ecx, eax;
    uint32_t	int_num, err_code;
    uint32_t	eip, cs, eflags, useresp, ss;
} __attribute__((packed));

typedef	struct	registers	registers_t;

// Use ON_NULLPTR macro to tell a user (developer) that he passed a nullptr
#ifndef RELEASE
#define ON_NULLPTR(ptr, code) \
	do {                         \
		if((ptr) == 0) { \
			qemu_err("You have an illusion that you see an object but you can't touch it because it's not exist..."); \
			code                                               \
		}      \
	} while(0)
#else
#define ON_NULLPTR(ptr, code)
#endif

#define WRITE32(reg, value) *(volatile uint32_t*)(hda_addr + (reg)) = (value)
#define READ32(reg) (*(volatile uint32_t*)(hda_addr + (reg)))
#define WRITE16(reg, value) *(volatile uint16_t*)(hda_addr + (reg)) = (value)
#define READ16(reg) (*(volatile uint16_t*)(hda_addr + (reg)))
#define WRITE8(reg, value) *(volatile uint8_t*)(hda_addr + (reg)) = (value)
#define READ8(reg) (*(volatile uint8_t*)(hda_addr + (reg)))
