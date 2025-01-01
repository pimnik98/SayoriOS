/**
 * @file io/ports.c
 * @author Пиминов Никита (nikita.piminoff@yandex.ru)
 * @brief Средства для работы с портами
 * @version 0.3.5
 * @date 2022-11-01
 * @copyright Copyright SayoriOS Team (c) 2022-2024
 */

#include <stdarg.h>
#include <io/ports.h>
#include <lib/sprintf.h>
#include "io/serial_port.h"
#include "sys/scheduler.h"
#include "mem/vmm.h"

void (*default_qemu_printf)(const char *text, ...) = qemu_printf;

void switch_qemu_logging() {
    default_qemu_printf = new_qemu_printf;
}

/**
 * @brief Чтение длинного слова через порт
 *
 * @param port - порт
 * @param buffer - данные
 * @param times - сколько данных прочесть
 */
void insl(uint16_t port, uint32_t *buffer, int32_t times) {
    for (uint32_t index = 0; index < times; index++) {
        buffer[index] = inl(port);
    }
}


/**
 * @brief Запись длинного слова через порт
 *
 * @param port - порт
 * @param buffer - данные
 * @param times - сколько данных отправить
 */
void outsl(uint16_t port, uint32_t *buffer, int32_t times) {
    for (int32_t index = 0; index < times; index++) {
        outl(port, buffer[index]);
    }
}

void insw(uint16_t __port, void *__buf, unsigned long __n) {
	__asm__ volatile("cld; rep; insw"
			: "+D"(__buf), "+c"(__n)
			: "d"(__port));
}
 
void outsw(uint16_t __port, const void *__buf, unsigned long __n) {
	__asm__ volatile("cld; rep; outsw"
			: "+S"(__buf), "+c"(__n)
			: "d"(__port));
}

/**
 * @brief Проверка занятости порта
 *
 * @return int32_t - состояние
 */
int32_t is_transmit_empty(uint16_t port) {
    return inb(port + 5) & 0x20;
}

// Read 1 byte (char) from port.
uint8_t serial_readchar(uint16_t port) {
   //size_t to = 0;
    while (is_signal_received(port) == 0){
       //to++;
       //qemu_warn("TIMEOUT: %d",to);
   }
   return inb(port);
}


// Read 1 byte (char) from port.
int8_t serial_readchar_timeout(uint16_t port,size_t timeout, bool Alert) {
    size_t to = 0;
    while (is_signal_received(port) == 0){
        to++;
        //qemu_warn("TIMEOUT: %d",to);
        if (to >= timeout){
            if (Alert) qemu_warn("TIMEOUT: %d",to);
            return -1;
        }
    }
    return inb(port);
}

/**
 * @brief Небольшая задержка используя порт 128(0x80)
 */
void io_wait(void) {
    outb(0x80, 0);
}

/**
 * @brief Проверка, читаем ли символ
 *
 * @param c Символ
 * @return 1 если читаемый, 0 если нет
 */
int isprint(char c) {
    return ((c >= ' ' && c <= '~') ? 1 : 0);
}


/**
 * @brief Проверка на тип порта
 *
 * @param port
 * @return Возвращает номер порта или 0 в случае если порт не в списке
 */
int is_com_port(int port) {
    switch (port) {
        case PORT_COM1:
            return 1;
        case PORT_COM2:
            return 2;
        case PORT_COM3:
            return 3;
        case PORT_COM4:
            return 4;
        case PORT_COM5:
            return 5;
        case PORT_COM6:
            return 6;
        case PORT_COM7:
            return 7;
        case PORT_COM8:
            return 8;
        default:
            return 0;
    }
}

/**
 * @brief Вывод QEMU через COM1 информации
 *
 * @param text Форматная строка
 * @param ... Дополнительные параметры
 */
void qemu_printf(const char *text, ...) {
    va_list args;
    va_start(args, text);

    if (__com_getInit(1)) {
        scheduler_mode(false);  // Stop scheduler

        __com_pre_formatString(PORT_COM1, text, args);

        scheduler_mode(true);  // Start scheduler
    }
    
    va_end(args);
}

void new_qemu_printf(const char *format, ...) {
    if (!__com_getInit(1))
        return;

    va_list args;
    va_start(args, format);

    char* container;

    vasprintf(&container, format, args);

    va_end(args);

    scheduler_mode(false);  // Stop scheduler
    __com_writeString(PORT_COM1, container);
    scheduler_mode(true);  // Start scheduler

    kfree(container);
}
