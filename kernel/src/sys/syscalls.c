/**
 * @file sys/syscalls.c
 * @author Пиминов Никита (nikita.piminoff@yandex.ru), NDRAEY >_ (pikachu_andrey@vk.com)
 * @brief Интерфейс системных вызовов
 * @version 0.3.4
 * @date 2022-10-01
 * @copyright Copyright SayoriOS Team (c) 2022-2023
 */

#include	"sys/syscalls.h"
#include	"io/ports.h"
#include	"io/tty.h"
#include	"user/env.h"
#include    <kernel.h>

syscall_fn_t* calls_table[NUM_CALLS] = {0};

/**
 * @brief Обработчик системных вызовов
 * 
 * @param regs - Регистр
 */
void syscall_handler(registers_t regs){
	qemu_log("syscall: %d", regs.eax);

	if (regs.eax >= NUM_CALLS) {
        qemu_err("Invalid system call!");
        return;
    }

	syscall_fn_t* entry_point = (syscall_fn_t*)calls_table[regs.eax];

	regs.eax = entry_point(regs.ebx, regs.ecx, regs.edx);
}

void syscall_env(struct env* position) {
    memcpy(position, &system_environment, sizeof(env_t));
}

void syscall_memory_alloc(size_t size, void** out) {
    void* allocated = kcalloc(size, 1);

    *out = allocated;
}

void syscall_memory_free(void* memory) {
    kfree(memory);
}

void syscall_tty_write(char* text) {
    _tty_puts(text);
}

/**
 * @brief Инициализация системных вызовов
 * 
 * @param regs - Регистр
 * @warning If every day goes like this; How do we survive?; We are working late on the night shift; To get peace of mind!
 */
void init_syscalls(void){
	register_interrupt_handler(0x50, &syscall_handler);

	// TODO: File operations using descriptors (do not pass FILE* structure!!!1)
	calls_table[0] = (syscall_fn_t *)syscall_env;
	calls_table[1] = (syscall_fn_t *)syscall_memory_alloc;
	calls_table[2] = (syscall_fn_t *)syscall_memory_free;
	calls_table[3] = (syscall_fn_t *)syscall_tty_write;

	qemu_ok("System calls initialized!");
}
