#pragma once

#include "common.h"

#define RDSP_PTR (char[8]){'R','S','D',' ','P','T','R',' '};

typedef struct RSDPDescriptor {
    char signature[8];
    uint8_t checksum;
    char OEMID[6];
    uint8_t revision;
    uint32_t RSDTaddress;
} RSDPDescriptor __attribute__ ((packed));