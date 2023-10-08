#include <kernel.h>
#include <sys/memory.h>
#include <sys/isr.h>

uint8_t rtl8139_busnum, rtl8139_slot, rtl8139_func;
uint32_t rtl8139_io_base, rtl8139_mem_base, rtl8139_bar_type;

size_t rtl8139_phys_buffer;
uint8_t rtl8139_irq;

uint8_t rtl8139_mac[6];

uint8_t TSAD_array[4] = {0x20, 0x24, 0x28, 0x2C};
uint8_t TSD_array[4] = {0x10, 0x14, 0x18, 0x1C};

size_t rtl8139_current_tx_index = 0;

#define NETCARD_NAME ("RTL8139")

void rtl8139_netcard_get_mac(uint8_t mac[6]) {
    rtl8139_read_mac();

    memcpy(mac, rtl8139_mac, 6);
}

netcard_entry_t rtl8139_netcard = {
    NETCARD_NAME,
    rtl8139_netcard_get_mac,
    rtl8139_send_packet
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

    uint16_t command_register = pci_read_confspc_word(rtl8139_busnum, rtl8139_slot, rtl8139_func, 4);

    command_register |= 0x05;

    pci_write(rtl8139_busnum, rtl8139_slot, rtl8139_func, 4, command_register);

    qemu_log("Enabled Bus Mastering for RTL8139!");

    uint32_t ret = pci_read_confspc_word(rtl8139_busnum, rtl8139_slot, rtl8139_func, 0x10);  // BAR0

    // If bar type is 0 use memory-based access, use port-based otherwise.
    rtl8139_bar_type = ret & 0x1;
    rtl8139_io_base = ret & (~0x3);
    rtl8139_mem_base = ret & (~0xf);

    qemu_log("RTL8139 BAR TYPE: %d; IO BASE: %x; MEMORY BASE: %x", rtl8139_bar_type, rtl8139_io_base, rtl8139_mem_base);

    rtl8139_wake_up();
    rtl8139_sw_reset();

    rtl8139_phys_buffer = alloc_phys_pages(RTL8139_BUFFER_PAGE_COUNT);

    map_pages(
        get_kernel_dir(),
        rtl8139_phys_buffer,
        rtl8139_phys_buffer,
        RTL8139_BUFFER_PAGE_COUNT,
        (PAGE_WRITEABLE | PAGE_PRESENT)
    );

    rtl8139_init_buffer();

    qemu_log("RTL8139 Physical buffer at: %x", rtl8139_phys_buffer);

    rtl8139_setup_rcr();
    rtl8139_enable_rx_tx();

    rtl8139_init_interrupts();

    rtl8139_read_mac();

    // If okay, add network card to list.
    netcard_add(rtl8139_netcard);
}

void rtl8139_read_mac() {
    uint32_t mac_part1 = inl(rtl8139_io_base + MAC0_5);
    uint16_t mac_part2 = ins(rtl8139_io_base + MAC0_5 + 4);

    rtl8139_mac[0] = (mac_part1 >> 0) & 0xFF;
    rtl8139_mac[1] = mac_part1 >> 8;
    rtl8139_mac[2] = mac_part1 >> 16;
    rtl8139_mac[3] = mac_part1 >> 24;

    rtl8139_mac[4] = mac_part2 >> 0;
    rtl8139_mac[5] = mac_part2 >> 8;

    qemu_log("Mac is: %v:%v:%v:%v:%v:%v", rtl8139_mac[0], rtl8139_mac[1], rtl8139_mac[2], rtl8139_mac[3], rtl8139_mac[4], rtl8139_mac[5]);
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
    
    while((inb(rtl8139_io_base + CMD) & 0x10) != 0);
}

void rtl8139_init_buffer() {
    outl(rtl8139_io_base + RBSTART, rtl8139_phys_buffer);
}

void rtl8139_handler(registers_t regs) {
    qemu_log("Received RTL8139 interrupt!");

    uint16_t status = ins(rtl8139_io_base + 0x3e);

    if(status & TOK) {
        qemu_log("Packet sent");
    }

    if (status & ROK) {
        qemu_log("Packet received!\n");
    }

    outs(rtl8139_io_base + 0x3E, 0x05);
}

void rtl8139_send_packet(void* data, size_t length) {
    // First, copy the data to a physically contiguous chunk of memory

    void* transfer_data = kmalloc_a(ALIGN(length, 4096));
    size_t phys_addr = virt2phys(get_kernel_dir(), (virtual_addr_t)transfer_data);
    memcpy(transfer_data, data, length);

    qemu_log("Send packet: Virtual memory at %x", transfer_data);
    qemu_log("Send packet: Physical memory at %x", phys_addr);

    // Second, fill in physical address of data, and length

    qemu_log("Before: %d",  rtl8139_current_tx_index);
    
    outl(rtl8139_io_base + TSAD_array[rtl8139_current_tx_index], (uint32_t)phys_addr);
    outl(rtl8139_io_base + TSD_array[rtl8139_current_tx_index++], length);
    
    qemu_log("After: %d",  rtl8139_current_tx_index);

    if(rtl8139_current_tx_index == 4)
        rtl8139_current_tx_index = 0;
}

size_t rtl8139_current_packet_ptr = 0;

void rtl8139_receive_packet() {
    uint16_t* packet = (uint16_t*)(rtl8139_phys_buffer + rtl8139_current_packet_ptr);
    // Skip packet header, get packet length
    uint16_t packet_header = packet[0];
    uint16_t packet_length = packet[1];

    uint16_t* packet_data = packet + 2;

    qemu_log("Packet!!!");
    qemu_log("Packet header: %x", packet_header);
    qemu_log("Packet length: %d", packet_length);

    qemu_log("Packet data: ");

    for(int i = 0; i < packet_length / sizeof(uint16_t); i++) {
        qemu_printf("%x", packet_data[i]);
    }

    qemu_printf("\n");

    rtl8139_current_packet_ptr = (rtl8139_current_packet_ptr + packet_length + 7) & RX_READ_POINTER_MASK;

    if(rtl8139_current_packet_ptr > 8192)
        rtl8139_current_packet_ptr -= 8192;

    outs(rtl8139_io_base + CAPR, rtl8139_current_packet_ptr - 0x10);
}

void rtl8139_init_interrupts() {
    outs(rtl8139_io_base + IMR, 0x0005);

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