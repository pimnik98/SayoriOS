/**
 * @file sys/syscalls.c
 * @author Пиминов Никита (nikita.piminoff@yandex.ru)
 * @brief Интерфейс системных вызовов
 * @version 0.3.0
 * @date 2022-10-01
 * @copyright Copyright SayoriOS Team (c) 2022
 */

#include	"sys/syscalls.h"
#include	"io/ports.h"
#include	"io/tty.h"

void hello_world_console() {
	qemu_log("Program says: Hello, World!!!");
}

/**
 * @brief Таблица системных вызовов
 */
void* calls_table[NUM_CALLS] = {
	/* Синхронизированный доступ к портам ввода/вывода*/
	&in_byte,
	&out_byte,
	&hello_world_console
};



/**
 * @brief Обработчик системных вызовов
 * 
 * @param registers_t regs - Регистр
 */
void syscall_handler(registers_t regs){
	if (regs.eax >= NUM_CALLS)
		return;

	void* syscall = calls_table[regs.eax];

	regs.eax = syscall_entry_call(syscall);
}
/**
 * @brief Инициализация системных вызовов
 * 
 * @param registers_t regs - Регистр
 */

void init_syscalls(void){
	register_interrupt_handler(0x50, &syscall_handler);
}

/**
 * @brief Получение цифры
 * 
 * @param int dig - Число
 * 
 * @return int - число
 * 
 * @warning Сделано для откладки
 */
int get_digit(int dig){
	return dig;
}

/**
 * @brief Список определенных системных вызовов
 */
DEFN_SYSCALL1(in_byte, PORT_INPUT_BYTE, uint16_t)
DEFN_SYSCALL2(out_byte, PORT_OUTPUT_BYTE, uint16_t, uint8_t)
DEFN_SYSCALL0(hello_world_console, HELLO_WORLD_CONSOLE)
