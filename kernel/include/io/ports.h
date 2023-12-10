#pragma once

#include <common.h>

#define PORT_COM1 0x3f8
#define PORT_COM2 0x2F8
#define PORT_COM3 0x3E8
#define PORT_COM4 0x2E8
#define PORT_COM5 0x5F8
#define PORT_COM6 0x4F8
#define PORT_COM7 0x5E8
#define PORT_COM8 0x4E8

#define LOG_WITH_TIME 0

#ifndef RELEASE
	#if LOG_WITH_TIME==0
		#define qemu_log(M, ...) qemu_printf("[LOG] (%s:%s:%d) " M "\n", __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__)
		#define qemu_warn(M, ...) qemu_printf("[\033[33;1mWARN\033[33;0m] (%s:%s:%d) \033[33;1m" M "\033[0m\n", __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__)
		#define qemu_err(M, ...) qemu_printf("[\033[31;1mERROR\033[33;0m] (%s:%s:%d) \033[31;1m" M "\033[0m\n", __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__)
	#else
		#define qemu_log(M, ...) qemu_printf("[LOG] [%f] (%s:%s:%d) " M "\033[0m\n", getUptime(), __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__)
		#define qemu_warn(M, ...) qemu_printf("[\033[33;1mWARN\033[33;0m] [%f] (%s:%s:%d) \033[33;1m" M "\033[0m\n", getUptime(), __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__)
		#define qemu_err(M, ...) qemu_printf("[\033[31;1mERR\033[33;0m] [%f] (%s:%s:%d) \033[31;1m" M "\033[0m\n", getUptime(), __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__)
	#endif
#else
	#define qemu_log(M, ...)
	#define qemu_warn(M, ...)
	#define qemu_err(M, ...)
#endif

#define assert(condition, format, ...) do { if (condition) {                 \
    qemu_printf("======================================\n");          \
    qemu_printf("ASSERT FAILED: " format "\n", ##__VA_ARGS__);   \
    qemu_printf("======================================\n");          \
    bsod_screen((registers_t){}, "ASSERT_FAIL", "See additional information on COM1 port. (Or Qemu.log if you're using QEMU)", 0xFFFF); \
}\
} while(0)

// Check if character received.
#define is_signal_received(port) (inb(port + 5) & 1)

#define outb(port, val) __asm__ volatile("outb %b0, %w1" : : "a"(val), "Nd"(port))
#define outw(port, val) outs(port, val)

#define fast_inb(port, ret) __asm__ volatile( "inb %1, %0" : "=a"(ret) : "Nd"((uint16_t)(port)) )
#define fast_ins(port, ret) __asm__ volatile( "inw %1, %0" : "=a"(ret) : "Nd"((uint16_t)(port)) )

/**
 * @brief Получение одного байта из порта
 *
 * @param port - порт
 * @return uint8_t - данные
 */
static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile ( "inb %1, %0"
                   : "=a"(ret)
                   : "Nd"(port) );
    return ret;

}
/**
 * @brief Чтение word из порта
 *
 * @param port - порт
 * @return uint16_t - word
 */
static inline uint16_t ins(uint16_t port) {
    uint16_t rv;
    asm volatile ("inw %1, %0" : "=a" (rv) : "Nd" (port));
    return rv;
}


/**
 * @brief Запись word в порт
 *
 * @param port - порт
 * @param data - данные
 */
static inline void outs(uint16_t port, uint16_t data) {
    asm volatile ("outw %1, %0" :: "Nd" (port), "a" (data));
}

void insw(uint16_t __port, void *__buf, unsigned long __n);
void outsw(uint16_t __port, const void *__buf, unsigned long __n);
void insl(uint16_t reg, uint32_t *buffer, int32_t quads);
void outsl(uint16_t reg, uint32_t *buffer, int32_t quads);

void outl(uint16_t port, uint32_t val);
uint32_t inl(uint16_t port);

int32_t com1_is_transmit_empty();
int is_com_port(int port);
void com1_write_char(char a);
void qemu_printf(const char *text, ...);
int32_t is_transmit_empty(uint16_t port);

// int32_t is_signal_received(uint16_t port);
uint8_t serial_readchar(uint16_t port);
void io_wait();

void new_qemu_printf(const char *format, ...);