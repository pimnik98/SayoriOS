/**
 * @brief Драйвер сетевой карты RTL8139
 * @author NDRAEY >_
 * @version 0.3.5
 * @date 2022-04-12
 * @copyright Copyright SayoriOS Team (c) 2022-2024
 */

#include <drv/rtl8139.h>
#include <net/cards.h>
#include <drv/pci.h>
#include <io/ports.h>
#include <net/endianess.h>
#include <net/ethernet.h>
#include <debug/hexview.h>
#include <sys/isr.h>
#include "lib/string.h"
#include "mem/vmm.h"
#include "mem/pmm.h"
#include "net/stack.h"

uint8_t rtl8139_busnum, rtl8139_slot, rtl8139_func;
uint32_t rtl8139_io_base, rtl8139_mem_base, rtl8139_bar_type;

size_t rtl8139_phys_buffer = 0;
char* rtl8139_virt_buffer = (char*)0;
uint8_t rtl8139_irq;

uint8_t rtl8139_mac[6];

uint8_t TSAD_array[4] = {0x20, 0x24, 0x28, 0x2C};
uint8_t TSD_array[4] = {0x10, 0x14, 0x18, 0x1C};

size_t rtl8139_current_tx_index = 0;

void* rtl8139_transfer_buffer;
size_t rtl8139_transfer_buffer_phys;

void rtl8139_send_packet(void* data, size_t length);
void rtl8139_receive_packet();

#define NETCARD_NAME ("RTL8139")

void rtl8139_netcard_get_mac(uint8_t mac[6]) {
	rtl8139_read_mac();

	memcpy(mac, rtl8139_mac, 6);
}

netcard_entry_t rtl8139_netcard = {
	NETCARD_NAME,
	{0, 0, 0, 0},
	rtl8139_netcard_get_mac,
	rtl8139_send_packet,
};

void rtl8139_init() {
	qemu_log("Initializing RTL8139...");

	pci_find_device(RTL8139_VENDOR, RTL8139_DEVICE,
					&rtl8139_busnum, &rtl8139_slot, &rtl8139_func);

	uint16_t devnum = pci_get_device(rtl8139_busnum, rtl8139_slot, rtl8139_func);

	qemu_log("RTL8139 ID: %d (%x)", devnum, devnum);

	if(devnum == PCI_VENDOR_NO_DEVICE) {
		qemu_log("RTL8139 is not connected!");
		return;
	} else {
		qemu_log("Detected RTL8139");
	}

	// Enable Bus Mastering

	pci_enable_bus_mastering(rtl8139_busnum, rtl8139_slot, rtl8139_func);

	qemu_log("Enabled Bus Mastering for RTL8139!");

	uint32_t ret = pci_read_confspc_word(rtl8139_busnum, rtl8139_slot, rtl8139_func, 0x10);  // BAR0

	// If bar type is 0 use memory-based access, use port-based otherwise.
	rtl8139_bar_type = ret & 0x1;
	rtl8139_io_base = ret & (~0x3);
	rtl8139_mem_base = ret & (~0xf);

	qemu_log("RTL8139 BAR TYPE: %d; IO BASE: %x; MEMORY BASE: %x", rtl8139_bar_type, rtl8139_io_base, rtl8139_mem_base);

	rtl8139_wake_up();
	rtl8139_sw_reset();

	rtl8139_virt_buffer = kmalloc_common(RTL8139_BUFFER_PAGE_COUNT, PAGE_SIZE);
	rtl8139_phys_buffer = virt2phys(get_kernel_page_directory(), (virtual_addr_t) rtl8139_virt_buffer);

	rtl8139_transfer_buffer = kmalloc_common(65535, PAGE_SIZE);
	rtl8139_transfer_buffer_phys = virt2phys(get_kernel_page_directory(), (virtual_addr_t)rtl8139_transfer_buffer);

	rtl8139_init_buffer();

	qemu_log("RTL8139 Physical buffer at: %x", rtl8139_phys_buffer);

	rtl8139_setup_rcr();
	rtl8139_enable_rx_tx();

	rtl8139_init_interrupts();

	rtl8139_read_mac();

	// If okay, add network card to list.
	netcard_add(&rtl8139_netcard);
}

void rtl8139_read_mac() {
	uint32_t mac_part1 = inl(rtl8139_io_base + MAC0_5);
	uint16_t mac_part2 = inw(rtl8139_io_base + MAC0_5 + 4);

	rtl8139_mac[0] = (mac_part1 >> 0) & 0xFF;
	rtl8139_mac[1] = mac_part1 >> 8;
	rtl8139_mac[2] = mac_part1 >> 16;
	rtl8139_mac[3] = mac_part1 >> 24;

	rtl8139_mac[4] = mac_part2 >> 0;
	rtl8139_mac[5] = mac_part2 >> 8;

	qemu_log("Mac is: %x:%x:%x:%x:%x:%x", rtl8139_mac[0], rtl8139_mac[1], rtl8139_mac[2], rtl8139_mac[3], rtl8139_mac[4], rtl8139_mac[5]);
}

void rtl8139_enable_rx_tx() {
	outb(rtl8139_io_base + CMD, 0x0C); // Sets the RE and TE bits high
}

void rtl8139_setup_rcr() {
	// AB - Accept Broadcast: Accept broadcast packets sent to mac ff:ff:ff:ff:ff:ff
	// AM - Accept Multicast: Accept multicast packets.
	// APM - Accept Physical Match: Accept packets send to NIC's MAC address.
	// AAP - Accept All Packets. Accept all packets (run in promiscuous mode). 
	// (1 << 7) is the WRAP bit, 0xf is AB+AM+APM+AAP

	outl(rtl8139_io_base + 0x44, 0xf | (1 << 7));
}

void rtl8139_wake_up() {
	outb(rtl8139_io_base + CONFIG_1, 0x0);
}

void rtl8139_sw_reset() {
	outb(rtl8139_io_base + CMD, 0x10);  // Reset
    
	while((inb(rtl8139_io_base + CMD) & 0x10) != 0)
		;
}

void rtl8139_init_buffer() {
	outl(rtl8139_io_base + RBSTART, rtl8139_phys_buffer);
}

void rtl8139_handler(__attribute__((unused)) registers_t regs) {
	qemu_log("Received RTL8139 interrupt!");

	uint16_t status = inw(rtl8139_io_base + 0x3e);

	if(status & TOK) {
		qemu_log("Packet sent");
	}

	if (status & ROK) {
		qemu_log("Packet received!\n");

		// TODO: Push packet to stack then, when needed, work with packets on the stack
		rtl8139_receive_packet();
	}

	outw(rtl8139_io_base + 0x3E, 0x05);
}

void rtl8139_send_packet(void *data, size_t length) {
	// First, copy the data to a physically contiguous chunk of memory

	memset(rtl8139_transfer_buffer, 0, 65535);

	memcpy(rtl8139_transfer_buffer, data, length);

	qemu_log("Send packet: Virtual memory at %x", (size_t)rtl8139_transfer_buffer);
	qemu_log("Send packet: Physical memory at %x", rtl8139_transfer_buffer_phys);

	// Second, fill in physical address of data, and length

	qemu_log("Before: %d",  rtl8139_current_tx_index);

	outl(rtl8139_io_base + TSAD_array[rtl8139_current_tx_index], (uint32_t)rtl8139_transfer_buffer_phys);
	outl(rtl8139_io_base + TSD_array[rtl8139_current_tx_index++], length);

	qemu_log("After: %d",  rtl8139_current_tx_index);

	if(rtl8139_current_tx_index > 3)
		rtl8139_current_tx_index = 0;

	qemu_log("Send packet end");
}

size_t rtl8139_current_packet_ptr = 0;

void rtl8139_receive_packet() {
	uint16_t* packet = (uint16_t*)(rtl8139_virt_buffer + rtl8139_current_packet_ptr);

	EthernetPacked* e_pack = (EthernetPacked*)packet;

	qemu_log("[Net] [RTL8139] Получен пакет!");
	qemu_log("  |-- Заголовок данных: %x",e_pack->Header);
	qemu_log("  |-- Длина данных: %d",e_pack->Size);
	qemu_log("  |-- Источник: %x:%x:%x:%x:%x:%x",e_pack->MAC_SOURCE[0] ,e_pack->MAC_SOURCE[1],e_pack->MAC_SOURCE[2], e_pack->MAC_SOURCE[3] ,e_pack->MAC_SOURCE[4] ,e_pack->MAC_SOURCE[5]);
	qemu_log("  |-- Кому: %x:%x:%x:%x:%x:%x", e_pack->MAC_DEVICE[0], e_pack->MAC_DEVICE[1], e_pack->MAC_DEVICE[2], e_pack->MAC_DEVICE[3], e_pack->MAC_DEVICE[4], e_pack->MAC_DEVICE[5]);
	qemu_log("  |-- Тип данных: %x", bit_flip_short(e_pack->Type));

	// Skip packet header, get packet length
	//^ Может быть и верно, но теряется сама логика пакета, из-за этого я не мог понять некоторые моменты
	// NDRAEY: Это сырая реализция!!!1

	uint16_t packet_header = e_pack->Header;
	uint16_t packet_length = e_pack->Size;

	uint16_t* packet_data = packet + 2;

	qemu_log("[Net] [RTL8139] Данные с пакета:\n");

//	hexview_advanced((char *) packet_data, packet_length, 10, true, new_qemu_printf);

	if(packet_header == HARDWARE_TYPE_ETHERNET) {
//        netstack_push(packet_data, packet_length);

		ethernet_handle_packet(&rtl8139_netcard, (ethernet_frame_t *) packet_data, packet_length);
	}

	rtl8139_current_packet_ptr = (rtl8139_current_packet_ptr + packet_length + 7) & RX_READ_POINTER_MASK;

	if(rtl8139_current_packet_ptr > 8192)
		rtl8139_current_packet_ptr -= 8192;

	outw(rtl8139_io_base + CAPR, rtl8139_current_packet_ptr - 0x10);
}

void rtl8139_init_interrupts() {
	outw(rtl8139_io_base + IMR, 0x0005);

	// https://wiki.osdev.org/PCI
	//
	// Offset 0x3C
	//
	// Bits 31-24 	Bits 23-16 	Bits 15-8   	Bits 7-0 
	// Max latency 	Min Grant 	Interrupt PIN 	Interrupt Line 
	//                                          ~~~~~~~~~~~~~~

	uint32_t word = pci_read_confspc_word(rtl8139_busnum, rtl8139_slot, rtl8139_func, 0x3C);  // All 0xF PCI register
	rtl8139_irq = word & 0xff;

	qemu_log("RTL8139 IRQ: %x", rtl8139_irq);

	register_interrupt_handler(32 + rtl8139_irq, rtl8139_handler);

	qemu_log("Initialized RTl8139 interrupts!");
}
