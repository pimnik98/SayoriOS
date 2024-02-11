#pragma once

#include "common.h"

#define AHCI_SIGNATURE_SATAPI 0xEB140101
#define AHCI_SIGNATURE_SATA 0x00000101

#define AHCI_HBA_ST 0x0001
#define AHCI_HBA_FRE 0x0010
#define AHCI_HBA_FR 0x4000
#define AHCI_HBA_CR 0x8000

#define AHCI_HBA_TFES (1 << 30)

typedef volatile struct {
	uint32_t command_list_base_address_low;  // 1K-byte aligned
	uint32_t command_list_base_address_high;
	uint32_t fis_base_address_low;  // 256-byte aligned
	uint32_t fis_base_address_high;
	uint32_t interrupt_status;
	uint32_t interrupt_enable;
	uint32_t command_and_status;
	uint32_t reserved;
	uint32_t task_file_data;
	uint32_t signature;
	uint32_t sata_status;
	uint32_t sata_control;
	uint32_t sata_error;
	uint32_t sata_active;
	uint32_t command_issue;
	uint32_t sata_notification;
	uint32_t fis_based_switch_control;

	uint32_t reserved1[11];
	uint32_t vendor[4];
} __attribute__((packed)) AHCI_HBA_PORT;

typedef volatile struct {
	uint32_t capability;
	uint32_t global_host_control;
	uint32_t interrupt_status;
	uint32_t port_implemented;
	uint32_t version;
	uint32_t command_completion_coalescing_control;
	uint32_t command_completion_coalescing_ports;
	uint32_t enclosure_management_location;
	uint32_t enclosure_management_control;
	uint32_t host_capabilities_extended;
	uint32_t handoff_control_and_status;

	char reserved[0xA0 - 0x2C];
	char vendor[0x100 - 0xA0];

	AHCI_HBA_PORT ports[0];
} __attribute__((packed)) AHCI_HBA_MEM;

typedef enum {
	FIS_TYPE_REG_HOST_TO_DEVICE	= 0x27,	// Register FIS - host to device
	FIS_TYPE_REG_DEVICE_TO_HOST	= 0x34,	// Register FIS - device to host
	FIS_TYPE_DMA_ACTIVATE		= 0x39,	// DMA activate FIS - device to host
	FIS_TYPE_DMA_SETUP			= 0x41,	// DMA setup FIS - bidirectional
	FIS_TYPE_DATA				= 0x46,	// Data FIS - bidirectional
	FIS_TYPE_BIST				= 0x58,	// BIST activate FIS - bidirectional
	FIS_TYPE_PIO_SETUP			= 0x5F,	// PIO setup FIS - device to host
	FIS_TYPE_DEV_BITS			= 0xA1,	// Set device bits FIS - device to host
} AHCI_FIS_TYPE;

typedef struct {
	uint8_t  fis_type;	// FIS_TYPE_REG_H2D

	uint8_t  pmport:4;	// Port multiplier
	uint8_t  rsv0:3;		// Reserved
	uint8_t  c:1;		// 1: Command, 0: Control

	uint8_t  command;	// Command register
	uint8_t  featurel;	// Feature register, 7:0

	uint8_t  lba0;		// LBA low register, 7:0
	uint8_t  lba1;		// LBA mid register, 15:8
	uint8_t  lba2;		// LBA high register, 23:16
	uint8_t  device;		// Device register

	uint8_t  lba3;		// LBA register, 31:24
	uint8_t  lba4;		// LBA register, 39:32
	uint8_t  lba5;		// LBA register, 47:40
	uint8_t  featureh;	// Feature register, 15:8

	uint8_t  countl;		// Count register, 7:0
	uint8_t  counth;		// Count register, 15:8
	uint8_t  icc;		// Isochronous command completion
	uint8_t  control;	// Control register

	uint8_t  rsv1[4];	// Reserved
} AHCI_FIS_REG_HOST_TO_DEVICE;

typedef struct {
	uint8_t  fis_type;    // FIS_TYPE_REG_D2H

	uint8_t  pmport:4;    // Port multiplier
	uint8_t  rsv0:2;      // Reserved
	uint8_t  i:1;         // Interrupt bit
	uint8_t  rsv1:1;      // Reserved

	uint8_t  status;      // Status register
	uint8_t  error;       // Error register

	uint8_t  lba0;        // LBA low register, 7:0
	uint8_t  lba1;        // LBA mid register, 15:8
	uint8_t  lba2;        // LBA high register, 23:16
	uint8_t  device;      // Device register

	uint8_t  lba3;        // LBA register, 31:24
	uint8_t  lba4;        // LBA register, 39:32
	uint8_t  lba5;        // LBA register, 47:40
	uint8_t  rsv2;        // Reserved

	uint8_t  countl;      // Count register, 7:0
	uint8_t  counth;      // Count register, 15:8
	uint8_t  rsv3[2];     // Reserved

	uint8_t  rsv4[4];     // Reserved
} AHCI_FIS_REG_DEVICE_TO_HOST;

typedef struct {
	// DWORD 0
	uint8_t  fis_type;	// FIS_TYPE_DATA

	uint8_t  pmport:4;	// Port multiplier
	uint8_t  rsv0:4;		// Reserved

	uint8_t  rsv1[2];	// Reserved

	// DWORD 1 ~ N
	uint32_t data[1];	// Payload
} AHCI_FIS_DATA;

typedef struct tagFIS_DMA_SETUP
{
	uint8_t  fis_type;	// FIS_TYPE_DMA_SETUP

	uint8_t  pmport:4;	// Port multiplier
	uint8_t  rsv0:1;		// Reserved
	uint8_t  d:1;		// Data transfer direction, 1 - device to host
	uint8_t  i:1;		// Interrupt bit
	uint8_t  a:1;            // Auto-activate. Specifies if DMA Activate FIS is needed

	uint8_t  rsved[2];       // Reserved

	uint32_t DMAbufferID_low;    // DMA Buffer Identifier. Used to Identify DMA buffer in host memory.
	uint32_t DMAbufferID_high;    // DMA Buffer Identifier. Used to Identify DMA buffer in host memory.

	uint32_t rsvd;           //More reserved

	uint32_t DMAbufOffset;   //Byte offset into buffer. First 2 bits must be 0
	uint32_t TransferCount;  //Number of bytes to transfer. Bit 0 must be 0

	uint32_t resvd;          //Reserved
} AHCI_FIS_DMA_SETUP;

typedef struct {
	uint8_t  fis_type;	// FIS_TYPE_PIO_SETUP

	uint8_t  pmport:4;	// Port multiplier
	uint8_t  rsv0:1;		// Reserved
	uint8_t  d:1;		// Data transfer direction, 1 - device to host
	uint8_t  i:1;		// Interrupt bit
	uint8_t  rsv1:1;

	uint8_t  status;		// Status register
	uint8_t  error;		// Error register

	uint8_t  lba0;		// LBA low register, 7:0
	uint8_t  lba1;		// LBA mid register, 15:8
	uint8_t  lba2;		// LBA high register, 23:16
	uint8_t  device;		// Device register

	uint8_t  lba3;		// LBA register, 31:24
	uint8_t  lba4;		// LBA register, 39:32
	uint8_t  lba5;		// LBA register, 47:40
	uint8_t  rsv2;		// Reserved

	uint8_t  countl;		// Count register, 7:0
	uint8_t  counth;		// Count register, 15:8
	uint8_t  rsv3;		// Reserved
	uint8_t  e_status;	// New value of status register

	uint16_t tc;		// Transfer count
	uint8_t  rsv4[2];	// Reserved
} AHCI_FIS_PIO_SETUP;

typedef struct {
	// 0
	uint8_t  cfl:5;		// Command FIS length in DWORDS, 2 ~ 16
	uint8_t  a:1;		// ATAPI
	uint8_t  w:1;		// Write, 1: H2D, 0: D2H
	uint8_t  p:1;		// Prefetchable

	// 1
	uint8_t  r:1;		// Reset
	uint8_t  b:1;		// BIST
	uint8_t  c:1;		// Clear busy upon R_OK
	uint8_t  rsv0:1;		// Reserved
	uint8_t  pmp:4;		// Port multiplier port

	// 2
	uint16_t prdtl;		// Physical region descriptor table length in entries

	// 4
	volatile uint32_t prdbc;		// Physical region descriptor byte count transferred

	// 8
	uint32_t ctba;		// Command table descriptor base address
	uint32_t ctbau;		// Command table descriptor base address upper 32 bits

	// 16
	uint32_t rsv1[4];	// Reserved
} AHCI_HBA_CMD_HEADER;

typedef struct {
	uint8_t fis_type;
	uint8_t pmport:4;
	uint8_t rsvd:2;
	uint8_t i:1;
	uint8_t n:1;
	uint8_t statusl:3;
	uint8_t rsvd2:1;
	uint8_t statush:3;
	uint8_t rsvd3:1;
	uint8_t error;
	uint32_t protocol;
} AHCI_FIS_DEV_BITS;


typedef volatile struct {
	AHCI_FIS_DMA_SETUP	dsfis;		// DMA Setup FIS
	uint8_t         pad0[4];

	AHCI_FIS_PIO_SETUP	psfis;		// PIO Setup FIS
	uint8_t         pad1[12];

	AHCI_FIS_REG_DEVICE_TO_HOST 	rfis;		// Register – Device to Host FIS
	uint8_t         pad2[4];

	AHCI_FIS_DEV_BITS	sdbfis;		// Set Device Bit FIS

	uint8_t         ufis[64];

	uint8_t   	rsv[0x100-0xA0];
} AHCI_HBA_FIS;

typedef struct {
	uint32_t dba;		// Data base address
	uint32_t dbau;		// Data base address upper 32 bits
	uint32_t rsv0;		// Reserved

	uint32_t dbc:22;		// Byte count, 4M max
	uint32_t rsv1:9;		// Reserved
	uint32_t i:1;		// Interrupt on completion
} __attribute__((packed)) AHCI_HBA_PRDT_ENTRY;

#define COMMAND_TABLE_PRDT_ENTRY_COUNT 8

typedef struct {
	uint8_t  cfis[64];	// Command FIS
	uint8_t  acmd[16];	// ATAPI command, 12 or 16 bytes
	uint8_t  rsv[48];	// Reserved
	AHCI_HBA_PRDT_ENTRY	prdt_entry[COMMAND_TABLE_PRDT_ENTRY_COUNT];	// Physical region descriptor table entries, 0 ~ 65535
} HBA_CMD_TBL;

#define COMMAND_LIST_ENTRY_COUNT 32
#define COMMAND_LIST_ENTRY_SIZE  sizeof(AHCI_HBA_CMD_HEADER)
#define COMMAND_LIST_SIZE (COMMAND_LIST_ENTRY_COUNT * COMMAND_LIST_ENTRY_SIZE)

#define FIS_SIZE sizeof(AHCI_HBA_FIS)

#define COMMAND_TABLE_ENTRY_SIZE sizeof(HBA_CMD_TBL)
#define COMMAND_TABLE_ENTRY_COUNT 32
#define COMMAND_TABLE_SIZE (COMMAND_TABLE_ENTRY_SIZE * COMMAND_LIST_ENTRY_COUNT)

#define MEMORY_PER_AHCI_PORT (COMMAND_LIST_SIZE + FIS_SIZE + COMMAND_TABLE_SIZE)

// Here we using port_num = 0 because we using new memory layout.
#define AHCI_COMMAND_LIST(mem, port_num) (((size_t)mem) + (MEMORY_PER_AHCI_PORT * port_num))
#define AHCI_FIS(mem, port_num) (AHCI_COMMAND_LIST(mem, port_num) + COMMAND_LIST_SIZE)
#define AHCI_COMMAND_TABLE(mem, port_num) (AHCI_FIS(mem, port_num) + FIS_SIZE)
#define AHCI_COMMAND_TABLE_ENTRY(mem, port_num, i) (AHCI_COMMAND_TABLE(mem, port_num) + (i * COMMAND_TABLE_ENTRY_SIZE))

struct ahci_port_descriptor {
    AHCI_HBA_CMD_HEADER* command_list_addr_virt;
    size_t command_list_addr_phys;

    size_t fis_virt;
    size_t fis_phys;
};

void ahci_init();
bool ahci_is_drive_attached(size_t port_num);
int ahci_free_cmd_slot(size_t port_num);
void ahci_start_cmd(size_t port_num);
void ahci_stop_cmd(size_t port_num);
void ahci_rebase_memory_for(size_t port_num);
void ahci_eject_cdrom(size_t port_num);
void ahci_read_sectors(size_t port_num, size_t location, size_t sector_count, void* buffer);
void ahci_write_sectors(size_t port_num, size_t location, size_t sector_count, void* buffer);
void ahci_identify(size_t port_num);