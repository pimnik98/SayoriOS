#include "drv/disk/ahci.h"

#include <lib/math.h>

#include "drv/pci.h"
#include "io/ports.h"
#include "io/tty.h"
#include "mem/pmm.h"
#include "mem/vmm.h"
#include "sys/isr.h"
#include "drv/disk/ata.h"
#include "drv/atapi.h"
#include "net/endianess.h"
#include "drv/disk/dpm.h"

#define AHCI_CLASS 1
#define AHCI_SUBCLASS 6

struct ahci_port_descriptor ports[32] = {0};

uint8_t ahci_busnum, ahci_slot, ahci_func;
uint16_t ahci_vendor = 0, ahci_devid = 0;
uint32_t ahci_irq;
bool ahci_initialized = false;

volatile AHCI_HBA_MEM* abar;

#undef qemu_log
#undef qemu_err
#undef qemu_warn
#undef qemu_ok
#define qemu_log(M, ...) tty_printf(M "\n", ##__VA_ARGS__)
#define qemu_err(M, ...) tty_printf("[ERR] " M "\n", ##__VA_ARGS__)
#define qemu_warn(M, ...) tty_printf("[WARN] " M "\n", ##__VA_ARGS__)
#define qemu_ok(M, ...) tty_printf("[OK] " M "\n", ##__VA_ARGS__)

#define AHCI_PORT(num) (abar->ports + (num))

//#define qemu_log(M, ...) _tty_printf(M "\n", ##__VA_ARGS__)

void ahci_irq_handler();

void ahci_init() {
	// Find controller

	pci_find_device_by_class_and_subclass(AHCI_CLASS, AHCI_SUBCLASS, &ahci_vendor, &ahci_devid, &ahci_busnum, &ahci_slot, &ahci_func);


	if(ahci_vendor == 0 || ahci_devid == 0) {
		qemu_err("AHCI contoller not found!");

		return;
	}

	qemu_ok("Found VEN: %x DEV: %x", ahci_vendor, ahci_devid);

	// Enable Bus Mastering
	uint16_t command_register = pci_read_confspc_word(ahci_busnum, ahci_slot, ahci_func, 4);

	command_register |= 0x05;

	pci_write(ahci_busnum, ahci_slot, ahci_func, 4, command_register);

//	qemu_ok("Enabled Bus Mastering");

	// Get ABAR

	abar = (volatile AHCI_HBA_MEM*)pci_read32(ahci_busnum, ahci_slot, ahci_func, 0x24);

	qemu_log("AHCI ABAR is: %x", abar);

	// Map memory
	map_pages(
            get_kernel_page_directory(),
            (physical_addr_t) abar,
            (virtual_addr_t) abar,
            PAGE_SIZE * 2,
            PAGE_WRITEABLE | PAGE_CACHE_DISABLE
    );

	qemu_log("Version: %x", abar->version);

    abar->global_host_control |= (1 << 31);  // AHCI Enable

    if(abar->host_capabilities_extended & 1) {
        for(int i = 0; i < 5; i++) {
            qemu_warn("PERFORMING BIOS HANDOFF!!!");
        }

        abar->handoff_control_and_status = abar->handoff_control_and_status | (1 << 1);

        while(1) {
            size_t status = abar->handoff_control_and_status;

            if (~status & (1 << 0))
                break;
        }
    } else {
        qemu_ok("No BIOS Handoff");
    }

	// Reset
// 	abar->global_host_control |= (1 << 0);
//
// 	while(abar->global_host_control & (1 << 0));

 	qemu_ok("Controller reset ok");

	// Interrupts
	ahci_irq = pci_read_confspc_word(ahci_busnum, ahci_slot, ahci_func, 0x3C) & 0xFF; // All 0xF PCI register
	qemu_log("AHCI IRQ: %x (%d)", ahci_irq, ahci_irq);

	register_interrupt_handler(32 + ahci_irq, ahci_irq_handler);

	// Init
	abar->global_host_control |= (1 << 31) | (1 << 1);  // AHCI Enable and AHCI Interrupts

	qemu_ok("Enabled AHCI and INTERRUPTS");

	size_t caps = abar->capability;
	size_t slotCount = ((caps >> 8) & 0x1f) + 1;

	qemu_log("Slot count: %d", slotCount);

	// Scan bus

	uint32_t implemented_ports = abar->port_implemented;

	for(int i = 0; i < 32; i++) {
		if (implemented_ports & (1 << i)) {
//			AHCI_HBA_PORT* port = AHCI_PORT(i);

			if (!ahci_is_drive_attached(i)) {
				continue;
			}

			/*
Ensure that the controller is not in the running state by reading and examining each
implemented port’s PxCMD register. If PxCMD.ST, PxCMD.CR, PxCMD.FRE and
PxCMD.FR are all cleared, the port is in an idle state. Otherwise, the port is not idle and
should be placed in the idle state prior to manipulating HBA and port specific registers.
System software places a port into the idle state by clearing PxCMD.ST and waiting for
PxCMD.CR to return ‘0’ when read. Software should wait at least 500 milliseconds for
this to occur. If PxCMD.FRE is set to ‘1’, software should clear it to ‘0’ and wait at least
500 milliseconds for PxCMD.FR to return ‘0’ when read. 
			*/

// 			port->command_and_status &= ~(1);
// 
// 			while(true) {
// 				uint32_t cr = (port->command_and_status >> 15) & 1;
// 				
// 				if(cr == 0) {
// 					break;
// 				}
// 			}
// 
// 			uint32_t fre = (port->command_and_status >> 4) & 1;
// 
// 			if(fre == 1) {
// 				port->command_and_status &= ~(1 << 4);
// 			}
// 
// 			while(true) {
// 				uint32_t fr = (port->command_and_status >> 14) & 1;
// 				
// 				if(fr == 0) {
// 					break;
// 				}
// 			}

            ahci_rebase_memory_for(i);

// 			port->command_and_status |= (1 << 4);
// 
// 			port->sata_error = (1 << i);
// 
//             port->interrupt_enable = (1 << 5) | (1 << 0) | (1 << 30) | (1 << 29) | (1 << 28) | (1 << 27) | (1 << 26) | (1 << 24) | (1 << 23);
// 
//             port->command_and_status |= 1;
        }
	}

	for(int i = 0; i < 32; i++) {
		if(abar->port_implemented & (1 << i)) {
			volatile AHCI_HBA_PORT* port = abar->ports + i;

			qemu_log("[%x: Port %d]", port, i);

			if(!ahci_is_drive_attached(i)) {
				qemu_log("\tNo drive attached to port!");
                continue;
            }

			if(port->signature == AHCI_SIGNATURE_SATAPI) { // SATAPI
				qemu_log("\tSATAPI drive");
                ahci_eject_cdrom(i);
			} else if(port->signature == AHCI_SIGNATURE_SATA) { // SATA
				qemu_log("\tSATA drive");
                ahci_identify(i);
			} else {
				qemu_log("Other device: %x", port->signature);
			}
		}
	}

	ahci_initialized = true;
}

void ahci_rebase_memory_for(size_t port_num) {
	if(port_num > 31)
		return;

	ahci_stop_cmd(port_num);

	// Memory rebase
	volatile AHCI_HBA_PORT* port = AHCI_PORT(port_num);

    void* virt = kmalloc_common(MEMORY_PER_AHCI_PORT, PAGE_SIZE);
    memset(virt, 0, MEMORY_PER_AHCI_PORT);

    // If gets laggy, comment it.
    phys_set_flags(get_kernel_page_directory(), (virtual_addr_t) virt, PAGE_WRITEABLE | PAGE_CACHE_DISABLE);

    size_t phys = virt2phys(get_kernel_page_directory(), (virtual_addr_t) virt);

    ports[port_num].command_list_addr_virt = virt;
    ports[port_num].command_list_addr_phys = phys;

    ports[port_num].fis_virt = AHCI_FIS(virt, 0);
    ports[port_num].fis_phys = AHCI_FIS(phys, 0);

//    qemu_log("Virtual addresses: Command list %x, FIS %x", ports[port_num].command_list_addr_virt, ports[port_num].fis_virt);

	port->command_list_base_address_low = phys;
	port->command_list_base_address_high = 0;

	port->fis_base_address_low = AHCI_FIS(phys, 0);
	port->fis_base_address_high = 0;

	AHCI_HBA_CMD_HEADER *cmdheader = (AHCI_HBA_CMD_HEADER*)virt;

	for(int i = 0; i < 32; i++) {
			cmdheader[i].prdtl = COMMAND_TABLE_PRDT_ENTRY_COUNT;

			cmdheader[i].ctba = AHCI_COMMAND_TABLE_ENTRY(phys, 0, i);

			cmdheader[i].ctbau = 0;
	}

//	qemu_log("Port %d", port_num);
//	qemu_log("\t|- CMD LIST BASE: %x (%s)", port->command_list_base_address_low, IS_ALIGNED(port->command_list_base_address_low, 1024) ? "aligned" : "not aligned");
//	qemu_log("\t|- FIS BASE: %x (%s)", port->fis_base_address_low, IS_ALIGNED(port->fis_base_address_low, 256) ? "aligned" : "not aligned");
//	qemu_log("\t|- TABLE ENTRIES: %x - %x", cmdheader[0].ctba, cmdheader[31].ctba + 256);

	ahci_start_cmd(port_num);

	qemu_ok("Rebasing memory for: %d is OK.", port_num);
}

bool ahci_is_drive_attached(size_t port_num) {
	if(port_num > 31){
		return false;
	}

	uint32_t implemented_ports = abar->port_implemented;

	if(implemented_ports & (1 << port_num)) {
		volatile AHCI_HBA_PORT* port = abar->ports + port_num;

		uint32_t status = port->sata_status;

		uint8_t ipm = (status >> 8) & 0xF;
		uint8_t det = status & 0xF;

		if(ipm == 1 && det == 3) {
			return true;
		}
	}

	return false;
}

int ahci_free_cmd_slot(size_t port_num) {
	if(port_num > 31)
		return -1;

	volatile AHCI_HBA_PORT* port = AHCI_PORT(port_num);

	uint32_t slots = port->sata_active | port->command_issue;

	for(int i = 0; i < 32; i++) {
		if((slots & (1 << i)) == 0)
			return i;
	}

	return -1;
}

void ahci_start_cmd(size_t port_num) {
	if(port_num > 31)
		return;

	volatile AHCI_HBA_PORT* port = AHCI_PORT(port_num);

	while (port->command_and_status & AHCI_HBA_CR);

	port->command_and_status |= AHCI_HBA_FRE;
	port->command_and_status |= AHCI_HBA_ST;
}

void ahci_stop_cmd(size_t port_num) {
	if(port_num > 31)
		return;

	volatile AHCI_HBA_PORT* port = AHCI_PORT(port_num);

	port->command_and_status &= ~AHCI_HBA_ST;
	port->command_and_status &= ~AHCI_HBA_FRE;

	while(1) {
		if (port->command_and_status & AHCI_HBA_FR)
			continue;
		if (port->command_and_status & AHCI_HBA_CR)
			continue;
		break;
	}
}

void ahci_irq_handler() {
	qemu_warn("AHCI interrupt!");

    uint32_t status = abar->interrupt_status;

    abar->interrupt_status = status;

    for(int i = 0; i < 32; i++) {
        if(status & (1 << i)) {
            volatile AHCI_HBA_PORT* port = AHCI_PORT(i);

            uint32_t port_interrupt_status = port->interrupt_status;

            port->interrupt_status = port_interrupt_status;

            // if(port_interrupt_status == 0) {
            //     continue;
            // }
        }
    }
}

void ahci_send_cmd(volatile AHCI_HBA_PORT *port, size_t slot) {
    int spin = 0;
    while ((port->task_file_data & (ATA_SR_BSY | ATA_SR_DRQ)) && spin < 1000000) {
        spin++;
    }

    if (spin == 1000000) {
        qemu_err("Port is hung");
        return;
    }

    qemu_warn("DRIVE IS READY");

    port->command_issue |= 1 << slot;

    qemu_warn("COMMAND IS ISSUED");

    while (1) {
        if (~port->command_issue & (1 << slot))  // Command is not running? Break
            break;

        if (port->interrupt_status & AHCI_HBA_TFES)	{  // Task file error? Tell about error and exit
            qemu_err("Read disk error (Task file error); IS: %x", port->interrupt_status);

            return;
        }
    }

    qemu_warn("OK");
}

/**
 * @brief Чтение `size` секторов с AHCI диска
 * @param port_num - номер порта
 * @param location - номер начального сектора
 * @param sector_count - колчество секторов
 * @param buffer - буфер куда сохранять данные
 */
void ahci_read_sectors(size_t port_num, size_t location, size_t sector_count, void* buffer) {
	if(!ahci_initialized) {
		qemu_err("AHCI not present!");
		return;
	}

	qemu_warn("\033[7mAHCI READ STARTED\033[0m");

	char* buffer_mem = kmalloc_common(sector_count * 512, PAGE_SIZE);
	memset(buffer_mem, 0, sector_count * 512);

	size_t buffer_phys = virt2phys(get_kernel_page_directory(), (virtual_addr_t) buffer_mem);

	volatile AHCI_HBA_PORT* port = AHCI_PORT(port_num);

	port->interrupt_status = (uint32_t)-1;

	AHCI_HBA_CMD_HEADER* hdr = ports[port_num].command_list_addr_virt;

	hdr->cfl = sizeof(AHCI_FIS_REG_DEVICE_TO_HOST) / sizeof(uint32_t);  // Should be 5
	hdr->a = 0;  // Not ATAPI
	hdr->w = 0;  // Read
	hdr->p = 0;  // No prefetch

	qemu_log("FIS IS %d DWORDs long", hdr->cfl);

	HBA_CMD_TBL* table = (HBA_CMD_TBL*)AHCI_COMMAND_TABLE(ports[port_num].command_list_addr_virt, 0);

	memset(table, 0, sizeof(HBA_CMD_TBL));

    size_t bytes = sector_count * 512;

	// FIXME: Simplify statements
	int index = 0;
	int i;
	for(i = 0; i < bytes; i += (4 * MB) - 1) {
		table->prdt_entry[index].dba = buffer_phys + i;
		table->prdt_entry[index].dbau = 0;
		table->prdt_entry[index].rsv0 = 0;
		table->prdt_entry[index].dbc = MIN((4 * MB), (bytes - i) % (4 * MB)) - 1;  // Size in bytes 4M max
		table->prdt_entry[index].rsv1 = 0;
		table->prdt_entry[index].i = 0;

		qemu_log("PRDT[%d]: Address: %x; Size: %d bytes; Last: %d",
			i,
			table->prdt_entry[index].dba,
			table->prdt_entry[index].dbc + 1,
			table->prdt_entry[index].i);

		index++;
	}

	table->prdt_entry[index - 1].i = 1;

    hdr->prdtl = index;

	AHCI_FIS_REG_HOST_TO_DEVICE *cmdfis = (AHCI_FIS_REG_HOST_TO_DEVICE*)&(table->cfis);

	qemu_log("CMDFIS at: %x", cmdfis);

	cmdfis->fis_type = FIS_TYPE_REG_HOST_TO_DEVICE;
	cmdfis->c = 1;	// Command
	cmdfis->command = ATA_CMD_READ_DMA_EXT;

	cmdfis->lba0 = location & 0xFF;
	cmdfis->lba1 = (location >> 8) & 0xFF;
	cmdfis->lba2 = (location >> 16) & 0xFF;
	cmdfis->device = 1 << 6;	// LBA mode

	cmdfis->lba3 = (location >> 24) & 0xFF;

#ifdef SAYORI64
    cmdfis->lba4 = (location >> 32) & 0xFF;
	cmdfis->lba5 = (location >> 40) & 0xFF;
#else
    cmdfis->lba4 = 0;
    cmdfis->lba5 = 0;
#endif

	cmdfis->countl = sector_count & 0xff;
	cmdfis->counth = (sector_count >> 8) & 0xff;

	ahci_send_cmd(port, 0);

	memcpy(buffer, buffer_mem, bytes);

	kfree(buffer_mem);

	qemu_warn("\033[7mOK?\033[0m");
}

/**
 * @brief Запись `size` секторов с AHCI диска
 * @param port_num - номер порта
 * @param location - номер начального сектора
 * @param sector_count - колчество секторов
 * @param buffer - буфер куда сохранять данные
 */
void ahci_write_sectors(size_t port_num, size_t location, size_t sector_count, void* buffer) {
	if(!ahci_initialized) {
		qemu_err("AHCI not present!");
		return;
	}

	qemu_warn("\033[7mAHCI WRITE STARTED\033[0m");

	char* buffer_mem = kmalloc_common(sector_count * 512, PAGE_SIZE);
	memset(buffer_mem, 0, sector_count * 512);
    memcpy(buffer_mem, buffer, sector_count * 512);

	size_t buffer_phys = virt2phys(get_kernel_page_directory(), (virtual_addr_t) buffer_mem);

	volatile AHCI_HBA_PORT* port = AHCI_PORT(port_num);

	port->interrupt_status = (uint32_t)-1;

	AHCI_HBA_CMD_HEADER* hdr = ports[port_num].command_list_addr_virt;

	hdr->cfl = sizeof(AHCI_FIS_REG_DEVICE_TO_HOST) / sizeof(uint32_t);  // Should be 5
	hdr->a = 0;  // Not ATAPI
	hdr->w = 1;  // Write
	hdr->p = 0;  // No prefetch

	qemu_log("FIS IS %d DWORDs long", hdr->cfl);

	HBA_CMD_TBL* table = (HBA_CMD_TBL*)AHCI_COMMAND_TABLE(ports[port_num].command_list_addr_virt, 0);

	memset(table, 0, sizeof(HBA_CMD_TBL));

    size_t bytes = sector_count * 512;

	// FIXME: Simplify statements
	int index = 0;
	int i;
	for(i = 0; i < bytes; i += (4 * MB) - 1) {
		table->prdt_entry[index].dba = buffer_phys + i;
		table->prdt_entry[index].dbau = 0;
		table->prdt_entry[index].rsv0 = 0;
		table->prdt_entry[index].dbc = MIN((4 * MB), (bytes - i) % (4 * MB)) - 1;  // Size in bytes 4M max
		table->prdt_entry[index].rsv1 = 0;
		table->prdt_entry[index].i = 0;

		qemu_log("PRDT[%d]: Address: %x; Size: %d bytes; Last: %d",
			i,
			table->prdt_entry[index].dba,
			table->prdt_entry[index].dbc + 1,
			table->prdt_entry[index].i);

		index++;
	}

	table->prdt_entry[index - 1].i = 1;

    hdr->prdtl = index;

	AHCI_FIS_REG_HOST_TO_DEVICE *cmdfis = (AHCI_FIS_REG_HOST_TO_DEVICE*)&(table->cfis);

	qemu_log("CMDFIS at: %x", cmdfis);

	cmdfis->fis_type = FIS_TYPE_REG_HOST_TO_DEVICE;
	cmdfis->c = 1;	// Command
	cmdfis->command = ATA_CMD_WRITE_DMA_EXT;

	cmdfis->lba0 = location & 0xFF;
	cmdfis->lba1 = (location >> 8) & 0xFF;
	cmdfis->lba2 = (location >> 16) & 0xFF;
	cmdfis->device = 1 << 6;	// LBA mode

	cmdfis->lba3 = (location >> 24) & 0xFF;

#ifdef SAYORI64
    cmdfis->lba4 = (location >> 32) & 0xFF;
	cmdfis->lba5 = (location >> 40) & 0xFF;
#else
    cmdfis->lba4 = 0;
    cmdfis->lba5 = 0;
#endif

	cmdfis->countl = sector_count & 0xff;
	cmdfis->counth = (sector_count >> 8) & 0xff;

	ahci_send_cmd(port, 0);

	kfree(buffer_mem);

	qemu_warn("\033[7mOK?\033[0m");
}


// Call SCSI START_STOP command to eject a disc
void ahci_eject_cdrom(size_t port_num) {
	qemu_log("Trying to eject %d", port_num);

	volatile AHCI_HBA_PORT* port = AHCI_PORT(port_num);

	port->interrupt_status = (uint32_t)-1;

	AHCI_HBA_CMD_HEADER* hdr = ports[port_num].command_list_addr_virt;

	hdr->cfl = sizeof(AHCI_FIS_REG_DEVICE_TO_HOST) / sizeof(uint32_t);  // Should be 5
	hdr->a = 1;  // ATAPI
	hdr->w = 0;  // Read
	hdr->p = 0;  // No prefetch
	hdr->prdtl = 0;  // No entries

	HBA_CMD_TBL* table = (HBA_CMD_TBL*)AHCI_COMMAND_TABLE(ports[port_num].command_list_addr_virt, 0);
	memset(table, 0, sizeof(HBA_CMD_TBL));

    uint8_t command[12] = {
        ATAPI_CMD_START_STOP,  // Command
        0, 0, 0,  // Reserved
        1 << 1, // Eject the disc
        0, 0, 0, 0, 0, 0, 0  // Reserved
    };

    memcpy(table->acmd, command, 12);

	volatile AHCI_FIS_REG_HOST_TO_DEVICE *cmdfis = (volatile AHCI_FIS_REG_HOST_TO_DEVICE*)&(table->cfis);
    memset((void*)cmdfis, 0, sizeof(AHCI_FIS_REG_HOST_TO_DEVICE));

	cmdfis->fis_type = FIS_TYPE_REG_HOST_TO_DEVICE;
	cmdfis->c = 1;	// Command
	cmdfis->command = ATA_CMD_PACKET;

    ahci_send_cmd(port, 0);
}

size_t ahci_dpm_read(size_t Disk, size_t Offset, size_t Size, void* Buffer){
    qemu_err("TODO: SATA DPM READ");

    DPM_Disk dpm = dpm_info(Disk + 65);

    return 0;
}

size_t ahci_dpm_write(size_t Disk, size_t Offset, size_t Size, void* Buffer){
    qemu_err("TODO: SATA DPM WRITE");

    DPM_Disk dpm = dpm_info(Disk + 65);

    return 0;
}


void ahci_identify(size_t port_num) {
    qemu_log("Identifying %d", port_num);

    volatile AHCI_HBA_PORT* port = AHCI_PORT(port_num);

    port->interrupt_status = (uint32_t)-1;

    int slot = 0;

    AHCI_HBA_CMD_HEADER* hdr = ports[port_num].command_list_addr_virt;
    hdr += slot;

    hdr->cfl = sizeof(AHCI_FIS_REG_DEVICE_TO_HOST) / sizeof(uint32_t);  // Should be 5
    hdr->a = 0;  // NOT ATAPI
    hdr->w = 0;  // Read
    hdr->p = 0;  // No prefetch
    hdr->prdtl = 1;  // One entry only

    void* memory = kmalloc_common(512, PAGE_SIZE);
    size_t buffer_phys = virt2phys(get_kernel_page_directory(), (virtual_addr_t) memory);
    memset(memory, 0, 512);

    HBA_CMD_TBL* table = (HBA_CMD_TBL*)AHCI_COMMAND_TABLE(ports[port_num].command_list_addr_virt, 0);
    memset(table, 0, sizeof(HBA_CMD_TBL));

    qemu_log("Table at: %x", table);

    // Set only first PRDT for testing
    table->prdt_entry[0].dba = buffer_phys;
    table->prdt_entry[0].dbc = 0x1ff;  // 512 bytes - 1
    table->prdt_entry[0].i = 0;

    volatile AHCI_FIS_REG_HOST_TO_DEVICE *cmdfis = (volatile AHCI_FIS_REG_HOST_TO_DEVICE*)&(table->cfis);

    cmdfis->fis_type = FIS_TYPE_REG_HOST_TO_DEVICE;
    cmdfis->c = 1;	// Command
    cmdfis->command = ATA_CMD_IDENTIFY;

    cmdfis->lba1 = 0;

    ahci_send_cmd(port, slot);

    uint16_t* memory16 = (uint16_t*)memory;

    uint16_t* model = kcalloc(20, 2);

    for(int i = 0; i < 20; i++) {
        model[i] = bit_flip_short(memory16[0x1b + i]);
    }

    *(((uint8_t*)model) + 39) = 0;


    size_t capacity = (memory16[101] << 16) | memory16[100];

    tty_printf("[SATA] MODEL: '%s';\n", model);
    tty_printf("[SATA] CAPACITY: %u sectors by 512 bytes;\n", capacity);

    int disk_inx = dpm_reg(
            (char)dpm_searchFreeIndex(0),
            "SATA Disk",
            "Unknown",
            1,
            capacity * 512,
            capacity,
            512,
            3, // Ставим 3ку, так как будем юзать функции для чтения и записи
            "DISK1234567890",
            (void*)0 // Оставим тут индекс диска
    );

    if (disk_inx < 0){
        qemu_err("[SATA/DPM] [ERROR] An error occurred during disk registration, error code: %d", disk_inx);
    } else {
        qemu_ok("[SATA/DPM] [Successful] Registering OK");
        dpm_fnc_write(disk_inx + 65, &ahci_dpm_read, &ahci_dpm_write);
    }


    kfree(memory);
    kfree(model);
}
