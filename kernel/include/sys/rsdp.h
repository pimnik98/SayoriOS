#pragma once

#include "common.h"

typedef struct RSDPDescriptor {
    char signature[8];
    uint8_t checksum;
    char OEMID[6];
    uint8_t revision;
    uint32_t RSDTaddress;
} __attribute__ ((packed)) RSDPDescriptor;

typedef struct RSDPDescriptorv2 {
    RSDPDescriptor firstPart;

    uint32_t Length;
    uint64_t XsdtAddress;
    uint8_t ExtendedChecksum;
    uint8_t reserved[3];
} __attribute__ ((packed)) RSDPDescriptorv2;