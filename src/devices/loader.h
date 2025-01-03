#pragma once

#define	FALSE			0
#define	TRUE			1

typedef enum {
    false = 0,
    true = 1
} bool;

#define nullptr ((void*)0)
#define NULL (0)
#define isUTF(c) (((c) & 0xFF80) == 0x0400)
#define ON_NULLPTR(ptr, code)

#if defined(ANDROID)
    /// Собрано под платформу Android
    #define CDEVICE 1
#elif defined(X86)
    /// Собрано под платформу X86
    #define CDEVICE 2
#else
    /// Собрано под платформу PSP
    #define CDEVICE 0
    #include "psp/psp.h"
    #include "psp/drivers/display.h"
#endif