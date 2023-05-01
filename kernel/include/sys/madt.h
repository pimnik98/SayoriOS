#pragma once

// https://wiki.osdev.org/MADT

#define MADT_SIGNATURE "APIC"
#include <common.h>

typedef struct {
	char signature[4];
	uint32_t length;
	char revision;
	char checksum;
	char oem_id[6];
	char oem_table$id[8];
	uint32_t oem_revision;
	uint32_t creator_id;
	uint32_t creator_revision;

	uint32_t lapic_addr;
	uint32_t flags;
} MADT_t;

// After the Flags field, starting at offset 0x2C,
// the rest of the MADT table contains a sequence
// of variable length records which enumerate the
// interrupt devices on this machine.

typedef enum {
	PROCESSOR_LOCAL_APIC = 0,
	IO_APIC = 1,
	IO_APIC_INTERRUPT_SOURCE = 2,
	LAPIC_ADDRESS_OVERRIDE = 5
} MADTEntryType;

typedef struct {
	char type;
	char length;
} MADTEntry_t;

typedef struct {
	char processor_id;
	char apic_id;
	uint32_t flags;
} ProcessorLocalAPIC_t;

typedef struct {
	char ioapic_id;
	char reserved;
	uint32_t ioapic_address;
	uint32_t global_system_interrupt_base;
} IOAPIC_t;

typedef struct {
	char bus_source;
	char irq_source;
	uint32_t global_system_interrupt;
	uint16_t flags;
} IOAPICInterruptSource_t;

