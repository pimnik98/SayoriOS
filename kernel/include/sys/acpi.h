#pragma once

#include "kernel.h"
#include "sys/rsdp.h"
#include "sys/rsdt.h"

extern uint32_t system_processors_found;

RSDPDescriptor* rsdp_find();
void find_facp(size_t rsdt_addr);
void find_apic(size_t rsdt_addr);