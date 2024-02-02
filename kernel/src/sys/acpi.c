#include "sys/acpi.h"
#include "sys/apic_table.h"
#include "sys/fadt.h"
#include "lib/string.h"
#include "io/ports.h"
#include "io/tty.h"
#include "mem/pmm.h"
#include "mem/vmm.h"

uint32_t system_processors_found = 0;

RSDPDescriptor* rsdp_find() {
    size_t saddr = 0x000E0000;
    char rsdp_ptr[8] = {'R','S','D',' ','P','T','R',' '};

	for(; saddr < 0x000FFFFF; saddr++) {
		if(memcmp((const char*)saddr, (const char*)rsdp_ptr, 8) == 0) {
			qemu_log("Found! At: %x", saddr);
            break;
		}
	}

    RSDPDescriptor* rsdp = (RSDPDescriptor*)saddr;

    // WORKS!
    //
    // char* a;
    // asprintf(&a, "RSDP sig: %.8s", rsdp->signature);
    // qemu_log("%s", a);
    // kfree(a);
    //
    // RSD PTR 

    qemu_log("RSDP sig: %.8s", rsdp->signature);
    qemu_log("RSDP checksum: %d", rsdp->checksum);
    qemu_log("RSDP OEMID: %s", rsdp->OEMID);
    qemu_log("RSDP revision: %d", rsdp->revision);
    qemu_log("RSDT address: %x", rsdp->RSDTaddress);

    return rsdp;
}

bool acpi_checksum_sdt(ACPISDTHeader *tableHeader) {
    unsigned char sum = 0;
 
    for (int i = 0; i < tableHeader->Length; i++) {
        sum += ((char*)tableHeader)[i];
    }
 
    return sum == 0;
}

ACPISDTHeader* find_table(uint32_t rsdt_addr, uint32_t sdt_count, char signature[4]) {
    uint32_t* rsdt_end = (uint32_t*)(rsdt_addr + sizeof(ACPISDTHeader));

    qemu_log("RSDT start: %x", rsdt_addr);
    qemu_log("RSDT end: %x", rsdt_end);
    qemu_log("RSDT size: %d", sizeof(ACPISDTHeader));

    for(uint32_t i = 0; i < sdt_count; i++) {
        ACPISDTHeader* entry = (ACPISDTHeader*)(rsdt_end[i]);

        if(strncmp(entry->Signature, signature, 4) == 0) {
            return entry;
        }
    }

    return 0;
}

void acpi_scan_all_tables(uint32_t rsdt_addr) {
    ACPISDTHeader* rsdt = (ACPISDTHeader*)rsdt_addr;

    map_pages(
        get_kernel_page_directory(),
        rsdt_addr,
		rsdt_addr,
        PAGE_SIZE * 2,
        PAGE_PRESENT
    );

    uint32_t sdt_count = (rsdt->Length - sizeof(ACPISDTHeader)) / sizeof(uint32_t);

    qemu_log("LEN: %d (// %d)", rsdt->Length, sizeof(ACPISDTHeader));

    uint32_t* rsdt_end = (uint32_t*)(rsdt_addr + sizeof(ACPISDTHeader));

    qemu_log("RSDT start: %x", rsdt_addr);
    qemu_log("RSDT end: %x", rsdt_end);
    qemu_log("RSDT size: %d", sizeof(ACPISDTHeader));

    for(uint32_t i = 0; i < sdt_count; i++) {
        ACPISDTHeader* entry = (ACPISDTHeader*)(rsdt_end[i]);

        if(entry == 0) {
            break;
        }

		tty_printf("[%x] Found table: %.4s\n", entry, entry->Signature);
		qemu_log("[%x] Found table: %.4s", entry, entry->Signature);
    }

    unmap_single_page(get_kernel_page_directory(), (virtual_addr_t) rsdt_addr);
    unmap_single_page(get_kernel_page_directory(), ((virtual_addr_t) rsdt_addr) + PAGE_SIZE);
}


void find_facp(size_t rsdt_addr) {
	qemu_log("FACP at P%x", rsdt_addr);

    map_pages(
        get_kernel_page_directory(),
        rsdt_addr,
		rsdt_addr,
        PAGE_SIZE,
        PAGE_PRESENT
    );


    ACPISDTHeader* rsdt = (ACPISDTHeader*)rsdt_addr;

//	new_qemu_printf("OEM: %.11s\n", rsdt->OEMID);
//	qemu_log("Length: %d", rsdt->Length);
//	new_qemu_printf("OEMTableID: %.8s\n", rsdt->OEMTableID);

    bool check = acpi_checksum_sdt(rsdt);

    qemu_log("Checksum: %s", check ? "PASS" : "FAIL");

    if(!check) {
        qemu_log("INVALID RSDT TABLE!");
        return;
    }

    qemu_log("OEMID: %s", rsdt->OEMID);
    qemu_log("Length: %d entries", rsdt->Length);

    uint32_t sdt_count = (rsdt->Length - sizeof(ACPISDTHeader)) / sizeof(uint32_t);

    qemu_log("SDTs available: %d", sdt_count);

    // Find FACP here

    ACPISDTHeader* pre_fadt = find_table((uint32_t) rsdt_addr, sdt_count, "FACP");

    if(!pre_fadt) {
        qemu_log("FADT not found...");
        return;
    }

    struct FADT* fadt = (struct FADT*)(pre_fadt + 1);  // It skips SDT header

//    map_single_page(get_kernel_page_directory(), (virtual_addr_t) fadt, (virtual_addr_t) fadt, PAGE_WRITEABLE);

    qemu_log("Century: %d", fadt->Century);
    qemu_log("SMI port: %x", fadt->SMI_CommandPort);
    qemu_log("Enable Command: %x", fadt->AcpiEnable);
    qemu_log("TODO: Write 'Enable Command' to 'SMI Port' using `outb()` func");

    qemu_log("Found FADT!");

//    unmap_single_page(get_kernel_page_directory(), (virtual_addr_t) fadt);
    unmap_single_page(get_kernel_page_directory(), (virtual_addr_t) rsdt_addr);
}

void find_apic(size_t rsdt_addr) {
// We have apic fail on real hardware, so I would to see logs on the screen
// #undef qemu_log
// #define qemu_log(M, ...) do { tty_printf(M, ##__VA_ARGS__); tty_puts("\n"); } while(0)

	qemu_log("!!! Starting RSDT: %x", rsdt_addr);

    // map_pages(
        // get_kernel_page_directory(),
        // rsdt_addr,
        // rsdt_addr,
        // PAGE_SIZE,
        // PAGE_PRESENT
    // );

	size_t start = rsdt_addr & ~0xfff;
	size_t end = ALIGN(rsdt_addr + PAGE_SIZE, PAGE_SIZE);

    map_pages(
        get_kernel_page_directory(),
        start,
        start,
        end - start,
        PAGE_PRESENT
    );

	qemu_log("!!! Should be: %x - %x", start, end);
	qemu_log("!!! Mapped memory range: %x - %x", rsdt_addr, rsdt_addr + (PAGE_SIZE));
    ACPISDTHeader* rsdt = (ACPISDTHeader*)rsdt_addr;

    bool check = acpi_checksum_sdt(rsdt);

    qemu_log("Checksum: %s", check ? "PASS" : "FAIL");

    if(!check) {
        qemu_log("INVALID RSDT TABLE!");
        return;
    }

    qemu_log("OEMID: %s", rsdt->OEMID);
    qemu_log("Length: %d entries", rsdt->Length);

    uint32_t sdt_count = (rsdt->Length - sizeof(ACPISDTHeader)) / sizeof(uint32_t);

    qemu_log("SDTs available: %d", sdt_count);

    // Find APIC here

    ACPISDTHeader* apic = find_table(rsdt_addr, sdt_count, "APIC");

    if(!apic) {
        qemu_log("APIC not found...");
        return;
    }

    qemu_log("Found APIC!");

    size_t table_end = (size_t)apic + sizeof(ACPISDTHeader);

	qemu_log("Table end at: %x", table_end);

    struct APIC_Base_Table* apic_base = (struct APIC_Base_Table*)table_end;

    qemu_log("LAPIC at: %x", apic_base->lapic_addr);
    qemu_log("Flags: %x", apic_base->flags);

    size_t base_table_end = table_end + sizeof(struct APIC_Base_Table);

    for(int i = 0; i < sdt_count; i++) {
        struct APIC_Entry* entry = (struct APIC_Entry*)base_table_end;

        qemu_log("[%x] Type: %d", entry, entry->type);

        switch(entry->type) {
            case APIC_PLAPIC: {
                qemu_log("PLAPIC!");

                qemu_log("- Processor ID: %d", entry->entry.plapic.processor_id);
                qemu_log("- Flags: %x", entry->entry.plapic.flags);
                qemu_log("- APIC ID: %x", entry->entry.plapic.apic_id);

				system_processors_found++;
                
                break;
            }

            case APIC_IOAPIC: {
                qemu_log("IOAPIC!");

                qemu_log("- ID: %d", entry->entry.ioapic.id);
                qemu_log("- IO APIC Address: %x", entry->entry.ioapic.io_apic_address);
                qemu_log("- Global System Interrupt Base: %x", entry->entry.ioapic.global_system_interrupt_base);
                
                break;
            }

            case APIC_IOAPIC_ISO: {
                qemu_log("IOAPIC Interrupt source override!");

                qemu_log("- Bus Source: %d", entry->entry.ioapic_iso.bus_source);
                qemu_log("- IRQ Source: %d", entry->entry.ioapic_iso.irq_source);
                qemu_log("- Global System Interrupt: %x", entry->entry.ioapic_iso.global_system_interrupt);
                qemu_log("- Flags: %x", entry->entry.ioapic_iso.flags);
                
                break;
            }

            case APIC_IOAPIC_NMI: {
                qemu_log("IOAPIC NMI!");

                qemu_log("- Source: %x", entry->entry.ioapic_nmi.source);
                qemu_log("- Global System Interrupt: %x", entry->entry.ioapic_nmi.global_system_interrupt);
                qemu_log("- Flags: %x", entry->entry.ioapic_nmi.flags);
                
                break;
            }

            case APIC_LAPIC_NMI: {
                qemu_log("LAPIC NMI!");

                qemu_log("- Processor ID: %d", entry->entry.lapic_nmi.processor_id);
                qemu_log("- LINT: %d", entry->entry.lapic_nmi.lint);
                qemu_log("- Flags: %x", entry->entry.lapic_nmi.flags);
                
                break;
            }

            default: {
                qemu_log("Unknown type! [%d]", entry->type);

                goto apic_detect_end;
            }
        }

        base_table_end += entry->record_length;
    }

    apic_detect_end:

    unmap_single_page(get_kernel_page_directory(), rsdt_addr);
}
