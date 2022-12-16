/**
 * @file sys/syscalls.c
 * @author Пиминов Никита (nikita.piminoff@yandex.ru)
 * @brief Интерфейс системных вызовов
 * @version 0.3.1
 * @date 2022-10-01
 * @copyright Copyright SayoriOS Team (c) 2022
 */

#include	"sys/syscalls.h"
#include	"io/ports.h"
#include	"io/tty.h"

#define TTY_CTL_PUTC 0x00
#define TTY_CTL_PUTS 0x01

#define ENV_SEND_DATA 		0x00		///< Отправка информации о структуре
#define ENV_TTY_CONTROL 	0x01		///< Управление виртуальным TTY для ENV


#define ENV_DATA_TTY_CREATE	0x00		///< Создание виртуального пространства

#define ENV_DATA_DEBUG_INT	0x00		///< Вывод в консоль %d
#define ENV_DATA_DEBUG_CHAR	0x01		///< Вывод в консоль %s
#define ENV_DATA_DEBUG_ADR	0x02		///< Вывод в консоль %x
#define ENV_DATA_DEBUG_FLO	0x03		///< Вывод в консоль %f



void tty_ctl(size_t function, size_t data) {
	if(function == TTY_CTL_PUTC) {
		tty_printf("%c", data);
	}else if(function == TTY_CTL_PUTS) {
		tty_printf("%s", (char*)data);
	}else{
		qemu_log("Unknown function: %x", function, data);
	}
	qemu_log("tty_ctl called with: FUNCTION: %x, DATA at: %x", function, (uint32_t)data);
}

/**
 * @brief Обработка команд окружения от ENV
 *
 * @warning Только для системных вызовов.
 */
void* sh_env(size_t function, size_t data){
	switch(function){
		case ENV_SEND_DATA:{
			return printEnv();
			break;
		}
		case ENV_TTY_CONTROL:{
			if (data == ENV_DATA_TTY_CREATE){
				qemu_log("[ENV] [TTY] [E:%x] Attempt to create a virtual address...", data);
				qemu_log("[ENV] [TTY] [E:%x] CRating space with size (%d)", data, getDisplaySize());
				uint8_t* backfb = kmalloc(getDisplaySize());
				memset(backfb, 0, getDisplaySize());
				memcpy(backfb, getDisplayAddr(), getDisplaySize());
				qemu_log("[ENV] [TTY] [E:%x] Space created at address (%x) with size (%d)",data,backfb,getDisplaySize());
				return backfb;
			}
			break;
		}
		default:{
			qemu_log("Unknown function: %d %d", function, data);
			return 0;
			break;
		}
	}
}

void sh_env_debug(void* dtr,void* data){
	if (dtr == ENV_DATA_DEBUG_INT){  		qemu_log("[ENV] [DEBUG] [E:%x] > %d",dtr,(int) data);	}
	else if (dtr == ENV_DATA_DEBUG_CHAR){ 	qemu_log("[ENV] [DEBUG] [E:%x] > %s",dtr,(char*) data);	}
	else if (dtr == ENV_DATA_DEBUG_ADR){  	qemu_log("[ENV] [DEBUG] [E:%x] > %x",dtr,(int) data);	}
	else { 									qemu_log("[ENV] [DEBUG] [E:%x] > Unknown data!"); 		}
	//if (dtr == ENV_DATA_DEBUG_FLO)  qemu_log("[ENV] [DEBUG] [E:%x] > (%f)",dtr,(double) data);
}

/**
 * @brief Таблица системных вызовов
 */
void* calls_table[NUM_CALLS] = {
	/* Синхронизированный доступ к портам ввода/вывода*/
	&in_byte,   	//0
	&out_byte,  	//1
	&tty_ctl,   	//2
	&sh_env,    	//3
	&sh_env_debug	//4
};

/**
 * @brief Обработчик системных вызовов
 * 
 * @param registers_t regs - Регистр
 */
void syscall_handler(registers_t regs){
	if (regs.eax >= NUM_CALLS)
		return;

	//qemu_log("Syscall: EAX: %x; EBX: %x; ECX: %x; EDX: %x", regs.eax, regs.ebx, regs.ecx, regs.edx);

	void* syscall = calls_table[regs.eax];

	//regs.eax = syscall_entry_call(syscall, regs.ebx, regs.ecx, regs.edx);
	regs.eax = ((size_t (*)(void*, void*, void*))syscall)(regs.ebx, regs.ecx, regs.edx);
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
DEFN_SYSCALL2(tty_ctl, TTYCTL, void*, void*)
DEFN_SYSCALL2(sh_env, SH_ENV, size_t, size_t)
DEFN_SYSCALL2(sh_env_debug, SH_ENV_DEBUG, void*, void*)
