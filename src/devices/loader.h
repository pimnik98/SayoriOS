#pragma once

#define	FALSE			0
#define	TRUE			1

#define nullptr ((void*)0)
#define NULL (0)

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
