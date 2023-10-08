/**
 * @file io/port_io.c
 * @author Пиминов Никита (nikita.piminoff@yandex.ru), NDRAEY >_ (pikachu_andrey@vk.com)
 * @brief Средства для работы с портами
 * @version 0.3.3
 * @date 2023-01-07
 * @copyright Copyright SayoriOS Team (c) 2022-2023
 */
#include <kernel.h>
#include <io/port_io.h>
#include <stdarg.h>

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

/**
 * @brief Получение одного байта из порта (Аналог inb)
 *
 * @param port - порт
 * @return uint8_t - данные
 */
uint8_t __com_readByte(uint16_t port){
    uint8_t ret;
    asm volatile ( "inb %1, %0"
                   : "=a"(ret)
                   : "Nd"(port) );
    return ret;
}

/**
 * @brief Запись одного байта в порт
 *
 * @param port - порт
 * @param val - данные
 */
void __com_writeByte(uint16_t port, uint8_t val){
    asm volatile("outb %1, %0" 
    : 
    : "dN"(port), 
      "a"(val)
    );
}

/**
 * @brief Запись 32х битного числа в порт
 *
 * @param port - порт
 * @param val - число
 */
void __com_writeInt32(uint16_t port, uint32_t val) {
    asm volatile ( "outl %0, %1" : : "a"(val), "Nd"(port) );
}

/**
 * @brief Чтение 32х битного числа
 *
 * @param port - порт
 * @return uint32_t - число
 */
uint32_t __com_readInt32(uint16_t port) {
    uint32_t ret;
    asm volatile ( "inl %1, %0"
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
uint16_t __com_readWord(uint16_t port) {
    uint16_t rv;
    asm volatile ("inw %1, %0" : "=a" (rv) : "dN" (port));
    return rv;
}


/**
 * @brief Запись word в порт
 *
 * @param port - порт
 * @param data - данные
 */
void __com_writeWord(uint16_t port, uint16_t data) {
    asm volatile ("outw %1, %0" : : "dN" (port), "a" (data));
}


/**
 * @brief Чтение длинного слова через порт
 *
 * @param port - порт
 * @param buffer - данные
 * @param times - сколько данных прочесть
 */
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
    return __com_readByte(port + 5) & 0x20;
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


void __com_writeInt(int16_t port, int32_t i){
    if (i < 0) {
        __com_writeChar(port,'-');
        i = -i;
    }
    
    uint32_t n, d = 1000000000;
    char str[255];
    uint32_t dec_index = 0;

    while ((i / d == 0) && (d >= 10)) {
        d /= 10;
    }
    n = i;

    while (d >= 10) {
        str[dec_index++] = ((char) ((int) '0' + n / d));
        n = n % d;
        d /= 10;
    }

    str[dec_index++] = ((char) ((int) '0' + n));
    str[dec_index] = 0;
    __com_writeString(port,str);
}

void __com_writeHex(int16_t port,uint32_t i,bool mode){
    const unsigned char hex[16]  =  { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
    uint32_t n, d = 0x10000000;

    if (mode) __com_writeString(port,"0x");

    while ((i / d == 0) && (d >= 0x10)) {
        d /= 0x10;
    }
    n = i;

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
                __com_writeString(port, string?string:"(nullptr)");
                break;
            case 'c':
                // FIXME: fix this! "warning: cast to pointer from integer of different size"
                __com_writeChar(port,(char)va_arg(args, int));
                break;
            case 'd':
                __com_writeInt(port,va_arg(args, int));
                break;
            case 'f': {
                double a = va_arg(args, double);
				if(!fpu_isInitialized()) {
					__com_writeString(port,"0.0000000");
					break;
				}

				if((int)a<0) {
					a = -a;
					__com_writeChar(port,'-');
				}
				
				double rem = a - (unsigned int)a;
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
                __com_writeInt(port,va_arg(args, unsigned int));
                break;
            case 'x':
                __com_writeHex(port,va_arg(args, uint32_t),true);
                break;
            case 'v':
                __com_writeHex(port,va_arg(args, uint32_t),false);
                break;
            default:
                __com_writeChar(port,format[i]);
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
    outb(port + 0, 0x01);    // Set divisor to 1 (lo byte) 115200 / divisor (1) = 115200 baud
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