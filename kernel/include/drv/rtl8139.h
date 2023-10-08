#pragma once

#define RTL8139_VENDOR 0x10EC
#define RTL8139_DEVICE 0x8139

#define RTL8139_BUFFER_PAGE_COUNT 4

enum RTL8139_regs {
    MAC0_5   = 0x00,  // 6 bytes long
    MAR0_7   = 0x08,  // 8 bytes long
    RBSTART  = 0x30,  // 4 bytes long
    CMD      = 0x37,  // 1 byte long
    IMR      = 0x3C,  // 2 bytes long
    ISR      = 0x3D,   // 2 bytes long
    CONFIG_1 = 0x52
};

#define ROK 1
#define TOK (1 << 2)
#define CAPR 0x38
#define RX_READ_POINTER_MASK (~3)

void rtl8139_init();
void rtl8139_wake_up();
void rtl8139_sw_reset();
void rtl8139_init_buffer();
void rtl8139_init_interrupts();
void rtl8139_read_mac();
void rtl8139_setup_rcr();
void rtl8139_enable_rx_tx();
void rtl8139_send_packet(void* data, size_t length);