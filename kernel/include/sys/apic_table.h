#pragma once

#include "common.h"

enum APIC_Type {
    APIC_PLAPIC = 0,
    APIC_IOAPIC = 1,
    APIC_IOAPIC_ISO = 2,
    APIC_IOAPIC_NMI = 3,
    APIC_LAPIC_NMI = 4,
    APIC_LAPIC_OVERRIDE = 5,
    APIC_PLx2APIC = 9
};

struct APIC_Base_Table {
    uint32_t lapic_addr;
    uint32_t flags;
} __attribute__((packed));

// Entry Type 0: Processor Local APIC
struct APIC_PLAPIC {
    uint8_t processor_id;
    uint8_t apic_id;
    uint32_t flags;
} __attribute__((packed));

// Entry Type 1: I/O APIC
struct APIC_IOAPIC {
    uint8_t id;
    uint8_t reserved;  // Should be 0
    uint32_t io_apic_address;
    uint32_t global_system_interrupt_base;
} __attribute__((packed));

// Entry Type 2: IO/APIC Interrupt source override
struct APIC_IOAPIC_ISO {
    uint8_t bus_source;
    uint8_t irq_source;
    uint32_t global_system_interrupt;
    uint16_t flags;
} __attribute__((packed));

// Entry Type 3: IO/APIC NMI Source
struct APIC_IOAPIC_NMI {
    uint8_t source;
    uint8_t reserved;  // Should be 0
    uint16_t flags;
    uint32_t global_system_interrupt;
} __attribute__((packed));

// Entry Type 4: Local APIC NMI Source
struct APIC_LAPIC_NMI {
    uint8_t processor_id;  // 0xFF = all processors
    uint16_t flags;
    uint8_t lint;  // Local Interrupt? (0 or 1)
} __attribute__((packed));

// Entry Type 5: Local APIC Address Override
struct APIC_LAPIC_OVERRIDE {
    uint16_t reserved;
    uint32_t lapic_phys_addr_low;
    uint32_t lapic_phys_addr_high;

    // On 64-bit systems
    // lapic_phys_addr = (lapic_phys_addr_low << 32) | lapic_phys_addr_high;
} __attribute__((packed));

// Entry Type 9: Processor Local x2APIC
struct APIC_PLx2APIC {
    uint16_t reserved;
    uint32_t processor_id;
    uint32_t flags;
    uint32_t acpi_id;
} __attribute__((packed));

struct APIC_Entry {
    uint8_t type;
    uint8_t record_length;

    union {
        struct APIC_PLAPIC plapic;
        struct APIC_IOAPIC ioapic;
        struct APIC_IOAPIC_ISO ioapic_iso;
        struct APIC_IOAPIC_NMI ioapic_nmi;
        struct APIC_LAPIC_NMI lapic_nmi;
        struct APIC_LAPIC_OVERRIDE lapic_override;
        struct APIC_PLx2APIC plx2apic;
    } entry;
} __attribute__((packed));