/**
 * @file io/port_io.c
 * @author Пиминов Никита (nikita.piminoff@yandex.ru), NDRAEY >_ (pikachu_andrey@vk.com)
 * @brief Средства для работы с портами
 * @version 0.3.5
 * @date 2023-01-07
 * @copyright Copyright SayoriOS Team (c) 2022-2024
 */

#include <lib/string.h>
#include <io/serial_port.h>
#include <stdarg.h>
#include "io/ports.h"
#include "drv/fpu.h"
#include "lib/math.h"

uint16_t com_init[8] = {0};    ///< Массив с инициализированными портами

/**
 * @brief Установка значения для инициализированного порта
 *
 * @param port - порт
 * @param value - данные
 */
void __com_setInit(uint16_t key, uint16_t value){
    com_init[key] = value;
}

/**
 * @brief Чтение значения для инициализированного порта
 *
 * @param key - порт
 *
 * @return uint16_t - данные
 */
uint16_t __com_getInit(uint16_t key){
    return com_init[key];
}

void __com_readBigData(uint16_t port, uint32_t *buffer, size_t times) {
    for (int32_t index = 0; index < times; index++) {
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
void __com_writeBigData(uint16_t port, uint32_t *buffer, size_t times) {
    for (int32_t index = 0; index < times; index++) {
        outl(port, buffer[index]);
    }
}

/**
 * @brief Чтение строки через порт
 *
 * @param port - порт
 * @param buf - данные
 * @param size - сколько данных прочесть
 */
void __com_readString(uint16_t port, uint32_t *buf, size_t size){
    __com_readBigData(port, buf, size);
}

/**
 * @brief Проверка занятости com-порта
 *
 * @return int32_t - состояние (если 0 - готов)
 */
int32_t __com_is_ready(uint16_t port){
    return inb(port + 5) & 0x20;
}

/**
 * @brief Отправка CHAR через COM-порт
 *
 * @param a - символ
 */
void __com_writeChar(uint16_t port,char a) {
    while (__com_is_ready(port) == 0);
    outb(port, a);
}

/*
* @brief Запись строки через порт
 *
 * @param port - Порт
 * @param buff - Строка
*/
void __com_writeString(uint16_t port, char *buf){
    for (size_t i = 0, len = strlen(buf); i < len; i++) {
        __com_writeChar(port, buf[i]);
    }
}

/**
 * @brief Небольшая задержка используя порт 128(0x80)
 * 
 * @test Тип нахуя?
 */
void __com_io_wait(){
    outb(0x80, 0);
}


void __com_writeInt(int16_t port, ssize_t i){
    char buffer[44] = {0};
    int index = 0;

    if(i == 0) {
        __com_writeChar(port, '0');
        return;
    }

    if(i < 0) {
        i = -i;
        __com_writeChar(port, '-');
    }

    while(i > 0) {
        buffer[index++] = '0' + (i % 10);
        i /= 10;
    }

    while(index--) {
        __com_writeChar(port, buffer[index]);
    }
}

void __com_writeUInt(int16_t port, size_t i){
    char buffer[44] = {0};
    int index = 0;

    while(i != 0) {
        buffer[index++] = '0' + (i % 10);
        i /= 10;
    }

    while(index--) {
        __com_writeChar(port, buffer[index]);
    }
}


void __com_writeHex(int16_t port, uint32_t i, bool mode){
    const unsigned char hex[16] = "0123456789ABCDEF";
    uint32_t n = i;
    uint32_t d = 0x10000000;

    if(mode)
        __com_writeString(port, "0x");

    while ((i / d == 0) && (d >= 0x10)) {
        d /= 0x10;
    }

    while (d >= 0xF) {
        __com_writeChar(port,(char)hex[n / d]);
        n = n % d;
        d /= 0x10;
    }
    __com_writeChar(port,(char)hex[n]);
}

void __com_pre_formatString(int16_t port, const char* format, va_list args){
    int32_t i = 0;
    char *string;

    while (format[i]) {
        if (format[i] == '%') {
            i++;
            switch (format[i]) {
            case 's':
                string = va_arg(args, char*);
                __com_writeString(port, string ? string : "(nullptr)");
                break;
            case 'c':
                __com_writeChar(port, va_arg(args, int));
                break;
            case 'd':
                __com_writeInt(port,va_arg(args, int));
                break;
            case 'f': {
                double a = va_arg(args, double);

				if(!fpu_isInitialized()) {
					__com_writeString(port,"!0.0000000");
					break;
				}

				if((int)a < 0) {
					a = -a;
					__com_writeChar(port,'-');
				}

				double rem = a - (int)a;
				__com_writeInt(port,(int)a);
				__com_writeChar(port,'.');
				
                for(int n = 0; n < 7; n++) {
				    __com_writeInt(
                        port,
                        (unsigned int)(rem * ipow(10, n + 1)) % 10
                    );
				}
            	
                break;
            }
            case 'i':
                __com_writeInt(port,va_arg(args, int));
                break;
            case 'u':
                __com_writeUInt(port,va_arg(args, unsigned int));
                break;
            case 'x':
                __com_writeHex(port,va_arg(args, uint32_t),true);
                break;
            case 'v':
                __com_writeHex(port,va_arg(args, uint32_t),false);
                break;
            default:
                __com_writeChar(port,format[i]);
				break;
            }
        } else {
            __com_writeChar(port,format[i]);
        }
        i++;
    }
}

/**
 * @brief Вывод через COM1 информации
 *
 * @param text Строка с параметрами
 */
void __com_formatString(int16_t port, char *text, ...) {
    va_list args;
    va_start(args, text);
    if (__com_getInit(1) == 1){
        __com_pre_formatString(port,text, args);
    }
    va_end(args);
}


/**
 * @brief Инициализация порта и проверка его доступности.
 */
int __com_init(uint16_t port) {
    outb(port + 1, 0x00);    // Disable all interrupts
    outb(port + 3, 0x80);    // Enable DLAB (set baud rate divisor)
    outb(port + 0, 0x03);    // Set divisor to 1 (lo byte) 115200 / divisor (1) = 115200 baud
    outb(port + 1, 0x00);    //                  (hi byte)
    outb(port + 3, 0x03);    // 8 bits, no parity, one stop bit
    outb(port + 2, 0xC7);    // Enable FIFO, clear them, with 14-byte threshold
    outb(port + 4, 0x0B);    // IRQs enabled, RTS/DSR set
    outb(port + 4, 0x1E);    // Set in loopback mode, test the serial chip
    outb(port + 0, 0xAE);    // Test serial chip (send byte 0xAE and check if serial returns same byte)

    if(inb(port + 0) != 0xAE) {
        return 1;
    }
        
    // If serial is not faulty set it in normal operation mode
    // (not-loopback with IRQs enabled and OUT#1 and OUT#2 bits enabled)
    outb(port + 4, 0x0F);

    return 0;
}