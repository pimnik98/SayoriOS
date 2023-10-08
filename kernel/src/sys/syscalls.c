/**
 * @file sys/syscalls.c
 * @author Пиминов Никита (nikita.piminoff@yandex.ru), NDRAEY >_ (pikachu_andrey@vk.com)
 * @brief Интерфейс системных вызовов
 * @version 0.3.3
 * @date 2022-10-01
 * @copyright Copyright SayoriOS Team (c) 2022-2023
 */

#include	"sys/syscalls.h"
#include	"io/ports.h"
#include	"io/tty.h"
#include	"user/env.h"
#include    <kernel.h>

#define TTY_CTL_PUTC 0x00
#define TTY_CTL_PUTS 0x01

#define SCREEN_PIXEL 0x00
#define SCREEN_UPDATE 0x01

#define ENV_SEND_DATA 		0x00		///< Отправка информации о структуре
#define ENV_TTY_CONTROL 	0x01		///< Управление виртуальным TTY для ENV

#define ENV_DATA_TTY_CREATE	0x00		///< Создание виртуального пространства

#define ENV_DATA_DEBUG_INT	0x00		///< Вывод в консоль %d
#define ENV_DATA_DEBUG_CHAR	0x01		///< Вывод в консоль %s
#define ENV_DATA_DEBUG_ADR	0x02		///< Вывод в консоль %x
#define ENV_DATA_DEBUG_FLO	0x03		///< Вывод в консоль %f

#define MEMCTL_ALLOCATE 0
#define MEMCTL_FREE     1

#define KBDCTL_KEY 0

#define TIMECTL_GETBOOTTICKS 0
#define TIMECTL_GETFREQ 1

void* calls_table[NUM_CALLS];

void tty_ctl(size_t function, size_t data) {
	qemu_log("Called TTY_CTL");
	if(function == TTY_CTL_PUTC) {
		tty_printf("%c", (char)data);
	}else if(function == TTY_CTL_PUTS) {
		// qemu_log("CALLED puts");
		tty_printf("%s", (char*)data);
	}
	// qemu_log("tty_ctl called with: FUNCTION: %x, DATA at: %x", function, (uint32_t)data);
}

void screen_ctl(size_t function, size_t data) {
	qemu_log("Fn is: %d", function);

	if(function == SCREEN_PIXEL) {
		screen_pixel* pix = (screen_pixel*)data;
		set_pixel(pix->x, pix->y, PACK_INTO_RGB(pix->color));
	}else if(function == SCREEN_UPDATE) {
		punch();
	}
}

/**
 * @brief Обработка команд окружения от ENV
 *
 * @warning Только для системных вызовов.
 */
void* sh_env(size_t function, size_t data){
	switch(function){
		case ENV_SEND_DATA: {
			env_t* from = printEnv();
			qemu_log("Address: DEST: %x; SRC: %x", data, from);
			memcpy((void*)data, from, sizeof(struct env));
			return 0;
		}
		case ENV_TTY_CONTROL: {
			if (data == ENV_DATA_TTY_CREATE){
				qemu_log("[ENV] [TTY] [E:%x] Attempt to create a virtual address...", data);
				qemu_log("[ENV] [TTY] [E:%x] CRating space with size (%d)", data, getDisplaySize());
				
				uint8_t* backfb = (uint8_t*)kmalloc(getDisplaySize());
				memset(backfb, 0, getDisplaySize());
				memcpy(backfb, (void*)getFrameBufferAddr(), getDisplaySize());
				
				qemu_log("[ENV] [TTY] [E:%x] Space created at address (%x) with size (%d)",data,backfb,getDisplaySize());
				return backfb;
			}
			return 0;
		}
		default:{
			qemu_log("Unknown function: %d %d", function, data);
			return 0;
		}
	}
}

void sh_env_debug(size_t dtr, void* data){
	if (dtr == ENV_DATA_DEBUG_INT)
		qemu_log("[ENV] [DEBUG] [E:%x] > %d",dtr,(int) data);
	else if (dtr == ENV_DATA_DEBUG_CHAR)
		qemu_log("[ENV] [DEBUG] [E:%x] > %s",dtr,(char*) data);
	else if (dtr == ENV_DATA_DEBUG_ADR)
		qemu_log("[ENV] [DEBUG] [E:%x] > %x",dtr,(int) data);
	else
		qemu_log("[ENV] [DEBUG] [E:%x] > Unknown data!");
}

void* memctl(size_t func, size_t* data, size_t parameter) {
	if(func == MEMCTL_ALLOCATE) {
		*data = (size_t)kmalloc(parameter);
	}else if(func == MEMCTL_FREE) {
		kfree(data);
	}

	return 0;
}

void kbdctl(size_t func, int* data) {
	if(func == KBDCTL_KEY) {
		*data = (int)getIntKeyboardWait();
	}
}

void timectl(size_t func, size_t* data) {
	if(func == TIMECTL_GETBOOTTICKS) {
		*data = getTicks();
	}else if(func == TIMECTL_GETFREQ) {
		*data = getFrequency();
	}
}

/**
 * @brief Обработчик системных вызовов
 * 
 * @param registers_t regs - Регистр
 */
void syscall_handler(registers_t regs){
	if (regs.eax >= NUM_CALLS)
		return;

	qemu_log("syscall: %d", regs.eax);

	void* syscall_fn = calls_table[regs.eax];
	size_t (*entry_point)(size_t, size_t, size_t) = (size_t (*)(size_t, size_t, size_t))syscall_fn;
	// qemu_log("Entry point at %x", entry_point);

	//regs.eax = syscall_entry_call(syscall, regs.ebx, regs.ecx, regs.edx);
	regs.eax = entry_point(regs.ebx, regs.ecx, regs.edx);

	// qemu_log("End syscall");
}

void empty_func() {}

/**
 * @brief Инициализация системных вызовов
 * 
 * @param registers_t regs - Регистр
 */

void init_syscalls(void){
	register_interrupt_handler(0x50, &syscall_handler);
	
	calls_table[0] = (void*)empty_func;
	calls_table[1] = (void*)empty_func;
	calls_table[2] = (void*)tty_ctl;
	calls_table[3] = (void*)sh_env;
	calls_table[4] = (void*)sh_env_debug;
	calls_table[5] = (void*)screen_ctl;
	calls_table[6] = (void*)memctl;
	calls_table[7] = (void*)kbdctl;
	calls_table[8] = (void*)timectl;
}


/**
 * @brief Список определенных системных вызовов
//  */
// DEFN_SYSCALL1(in_byte, PORT_INPUT_BYTE, uint16_t)
// DEFN_SYSCALL2(out_byte, PORT_OUTPUT_BYTE, uint16_t, uint8_t)
// DEFN_SYSCALL2(tty_ctl, TTYCTL, void*, void*)
// DEFN_SYSCALL2(sh_env, SH_ENV, size_t, size_t)
// DEFN_SYSCALL2(sh_env_debug, SH_ENV_DEBUG, void*, void*)
// DEFN_SYSCALL2(screen_ctl, SCREENCTL, size_t, size_t)
// DEFN_SYSCALL3(memctl, MEMCTL, size_t, void*, size_t)
