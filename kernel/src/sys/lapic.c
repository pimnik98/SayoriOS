//
// Created by ndraey on 2/13/24.
//

#include "common.h"
#include "sys/rsdp.h"
#include "sys/acpi.h"
#include "io/ports.h"

void lapic_init(RSDPDescriptor *rsdp) {
    size_t lapic_addr;

    find_apic(rsdp->RSDTaddress, &lapic_addr);

    qemu_log("LAPIC AT: %x", lapic_addr);
}