#pragma once

#include <common.h>
#include <sys/timer.h>

#define PORT_COM1 0x3f8
#define PORT_COM2 0x2F8
#define PORT_COM3 0x3E8
#define PORT_COM4 0x2E8
#define PORT_COM5 0x5F8
#define PORT_COM6 0x4F8
#define PORT_COM7 0x5E8
#define PORT_COM8 0x4E8

#define LOG_WITH_TIME 0

extern void (*default_qemu_printf)(const char *text, ...);

#ifndef RELEASE
	#if LOG_WITH_TIME == 0
		#define qemu_log(M, ...) default_qemu_printf("[LOG] (%s:%s:%d) " M "\n", __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__)
		#define qemu_note(M, ...) default_qemu_printf("[\033[36;1mNOTE\033[33;0m] (%s:%s:%d) \033[36;1m" M "\033[0m\n", __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__)
		#define qemu_warn(M, ...) default_qemu_printf("[\033[33;1mWARN\033[33;0m] (%s:%s:%d) \033[33;1m" M "\033[0m\n", __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__)
		#define qemu_ok(M, ...) default_qemu_printf("[\033[32;1mOK\033[33;0m] (%s:%s:%d) \033[32;1m" M "\033[0m\n", __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__)
		#define qemu_err(M, ...) default_qemu_printf("[\033[31;1mERROR\033[33;0m] (%s:%s:%d) \033[31;1m" M "\033[0m\n", __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__)
	#else
		#define qemu_log(M, ...) qemu_printf("[LOG] [%d] (%s:%s:%d) " M "\033[0m\n", timestamp(), __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__)
		#define qemu_note(M, ...) qemu_printf("[\033[36;1mWARN\033[33;0m] [%d] (%s:%s:%d) \033[36;1m" M "\033[0m\n", timestamp(), __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__)
		#define qemu_warn(M, ...) qemu_printf("[\033[33;1mWARN\033[33;0m] [%d] (%s:%s:%d) \033[33;1m" M "\033[0m\n", timestamp(), __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__)
		#define qemu_ok(M, ...) qemu_printf("[\033[32;1mOK\033[33;0m] [%d] (%s:%s:%d) \033[32;1m" M "\033[0m\n", timestamp(), __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__)
		#define qemu_err(M, ...) qemu_printf("[\033[31;1mERR\033[33;0m] [%d] (%s:%s:%d) \033[31;1m" M "\033[0m\n", timestamp(), __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__)
	#endif
#else
    #define qemu_note(M, ...)
    #define qemu_log(M, ...)
	#define qemu_warn(M, ...)
	#define qemu_ok(M, ...)
	#define qemu_err(M, ...)
#endif

#define assert(condition, format, ...) do { if (condition) {                 \
    qemu_printf("======================================\n");          \
    qemu_printf("ASSERT FAILED: " format "\n", ##__VA_ARGS__);   \
    qemu_printf("======================================\n");          \
    bsod_screen((registers_t){}, "ASSERT_FAIL", "See additional information on COM1 port. (Or Qemu.log if you're using QEMU)", 0xFFFF); \
} } while(0)

// Check if character received.
#define is_signal_received(port) (inb(port + 5) & 1)

/**
 * @brief Отправка одного байта в порт
 *
 * @param port - порт
 * @param val - данные
 */
SAYORI_INLINE void outb(uint16_t port, uint8_t val) {
	__asm__ volatile("outb %b0, %w1" : : "a"(val), "Nd"(port));
}

/**
 * @brief Получение одного байта из порта
 *
 * @param port - порт
 * @return Байт из порта
 */
SAYORI_INLINE uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ( "inb %1, %0"
                   : "=a"(ret)
                   : "Nd"(port) );
    return ret;

}

/**
 * @brief Запись 32-битного слова в порт
 *
 * @param port - порт
 * @param val - число
 */
SAYORI_INLINE void outl(uint16_t port, uint32_t val) {
	__asm__ volatile ( "outl %0, %1" : : "a"(val), "Nd"(port) );
}

/**
 * @brief Чтение 32-битного слова
 *
 * @param port - порт
 * @return Слово из порта
 */
SAYORI_INLINE uint32_t inl(uint16_t port) {
	uint32_t ret;
	__asm__ volatile( "inl %1, %0" : "=a"(ret) : "Nd"(port) );
	return ret;
}

/**
 * @brief Чтение 16-битного слова из порта
 *
 * @param port - порт
 * @return Слово из порта
 */
SAYORI_INLINE uint16_t inw(uint16_t port) {
    uint16_t ret;
    __asm__ volatile ("inw %1, %0" : "=a" (ret) : "Nd" (port));
    return ret;
}

/**
 * @brief Запись 16-битного слова в порт
 *
 * @param port - порт
 * @param data - данные
 */
SAYORI_INLINE void outw(uint16_t port, uint16_t data) {
    __asm__ volatile ("outw %1, %0" :: "Nd" (port), "a" (data));
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

uint8_t serial_readchar(uint16_t port);
void io_wait();

void new_qemu_printf(const char *format, ...);