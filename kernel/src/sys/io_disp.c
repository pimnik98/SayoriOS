/**
 * @file sys/io_disp.c
 * @author Пиминов Никита (nikita.piminoff@yandex.ru)
 * @brief I/O Dispatcher
 * @version 0.3.0
 * @date 2022-10-01
 * @copyright Copyright SayoriOS Team (c) 2022
 */
#include	"sys/io_disp.h"
mutex_t*	port_mutex;

/**
 * @brief Инициализация I/O Dispatcher
 */
void init_io_dispatcher(void){
	int i = 0;
	port_mutex = (mutex_t*) kmalloc(sizeof(mutex_t)*PORTS_NUM);	///< Выделить память для мьютексов порта и освободить их
	for (i = 0; i < PORTS_NUM; i++){
		mutex_release(&port_mutex[i]);
	}
}

/**
 * @brief Прочитать данные с порта
 * 
 * @param uint16_t port - Порт
 *
 * @return uint8_t - Данные с порта
 */
uint8_t in_byte(uint16_t port){
	uint8_t value = 0;
	mutex_get(&port_mutex[port], true);
	value = inb(port);
	mutex_release(&port_mutex[port]);
	return value;
}

/**
 * @brief Записать данные в порт
 * 
 * @param uint16_t port - Порт
 * @param uint8_t value - Значение
 */
void out_byte(uint16_t port, uint8_t value){
	mutex_get(&port_mutex[port], true);
	outb(port, value);
	mutex_release(&port_mutex[port]);
}

/**
 * @brief Прочитать данные с порта
 *
 * @param uint16_t port - Порт
 *
 * @return uint16_t - Данные с порта
 */
uint8_t in_word(uint16_t port){
    uint8_t value = 0;
    mutex_get(&port_mutex[port], true);
    value = ins(port);
    mutex_release(&port_mutex[port]);
    return value;
}

/**
 * @brief Записать данные в порт
 *
 * @param uint16_t port - Порт
 * @param uint16_t value - Значение
 */
void out_word(uint16_t port, uint8_t value){
    mutex_get(&port_mutex[port], true);
    outs(port, value);
    mutex_release(&port_mutex[port]);
}
