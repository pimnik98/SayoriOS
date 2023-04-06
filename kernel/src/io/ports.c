/**
 * @file io/ports.c
 * @author Пиминов Никита (nikita.piminoff@yandex.ru)
 * @brief Средства для работы с портами
 * @version 0.3.2
 * @date 2022-11-01
 * @copyright Copyright SayoriOS Team (c) 2022-2023
 */
#include <kernel.h>
#include <stdarg.h>
#include <io/ports.h>

/**
 * @brief Запись 32х битного числа в порт
 *
 * @param port - порт
 * @param val - число
 */
void outl(uint16_t port, uint32_t val) {
    asm volatile ( "outl %0, %1" : : "a"(val), "Nd"(port) );
}


/**
 * @brief Чтение 32х битного числа
 *
 * @param port - порт
 * @return uint32_t - число
 */
uint32_t inl(uint16_t port) {
    uint32_t ret;
    asm volatile ( "inl %1, %0"
                   : "=a"(ret)
                   : "Nd"(port) );
    return ret;
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
   while (is_signal_received(port) == 0);
   return inb(port);
}

/**
 * @brief Небольшая задержка используя порт 128(0x80)
 */
void io_wait(void) {
    outb(0x80, 0);
}

/**
 * @brief Чтение 16х битного числа
 *
 * @param port - порт
 * @return uint16_t - число
 */
uint16_t inw(uint16_t port){
	uint16_t ret;
	asm volatile ("inw %1, %0":"=a"(ret):"dN"(port));
	return ret;
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
            break;
        case PORT_COM2:
            return 2;
            break;
        case PORT_COM3:
            return 3;
            break;
        case PORT_COM4:
            return 4;
            break;
        case PORT_COM5:
            return 5;
            break;
        case PORT_COM6:
            return 6;
            break;
        case PORT_COM7:
            return 7;
            break;
        case PORT_COM8:
            return 8;
            break;
        default:
            return 0;
            break;
    }
}

/**
 * @brief Вывод QEMU через COM1 информации
 *
 * @param text Строка с параметрами
 */
void qemu_printf(char *text, ...) {
    va_list args;
    va_start(args, text);
    if (__com_getInit(1) == 1) {
        __com_pre_formatString(PORT_COM1, text, args);
    }
    va_end(args);
}
