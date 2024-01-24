/**
 * @file sys/syscalls.c
 * @author Пиминов Никита (nikita.piminoff@yandex.ru), NDRAEY >_ (pikachu_andrey@vk.com)
 * @brief Интерфейс системных вызовов
 * @version 0.3.5
 * @date 2022-10-01
 * @copyright Copyright SayoriOS Team (c) 2022-2024
 */

#include	"sys/syscalls.h"
#include	"io/ports.h"
#include	"io/tty.h"
#include	"user/env.h"
#include    "sys/file_descriptors.h"
#include    <kernel.h>
#include    <sys/trigger.h>

syscall_fn_t* calls_table[NUM_CALLS] = {0};

/**
 * @brief Обработчик системных вызовов
 * 
 * @param regs - Регистр
 */
void syscall_handler(volatile registers_t regs) {
//	qemu_log("syscall: %d", regs.eax);

	if (regs.eax >= NUM_CALLS) {
        qemu_err("Invalid system call: %d!", regs.eax);

        __asm__ volatile("movl %0, %%eax" :: "r"(0));
        return;
    }

	syscall_fn_t* entry_point = (syscall_fn_t*)calls_table[regs.eax];

	regs.eax = entry_point(regs.ebx, regs.ecx, regs.edx);

    // TODO: Just place result into eax, I know how to do it!

    __asm__ volatile("movl %0, %%eax" :: "r"(regs.eax));
}

size_t syscall_env(struct env* position) {
    memcpy(position, &system_environment, sizeof(env_t));

    return 0;
}

size_t syscall_memory_alloc(size_t size, void** out) {
    void* allocated = kcalloc(size, 1);

    *out = allocated;

    return 0;
}

size_t syscall_memory_free(void* memory) {
    kfree(memory);

    return 0;
}

size_t syscall_tty_write(char* text) {
    tty_puts(text);
    return 0;
}

int syscall_trigger_reg(int type,trigger_cmd_t handler){
    int init = RegTrigger(type,handler);
    qemu_warn("[SysCall] [Triggers] REG -> I:%x | T:%x",init, type);
    return init;
}

size_t syscall_trigger_on(int index){
    qemu_warn("[SysCall] [Triggers] ON -> I:%x",index);
    OnTrigger(index);
    return 1;
}

size_t syscall_trigger_off(int index){
    qemu_warn("[SysCall] [Triggers] OFF -> I:%x",index);
    OffTrigger(index);
    return 1;
}

size_t syscall_trigger_del(int index){
    qemu_warn("[SysCall] [Triggers] Delete -> I:%x",index);
    DeleteTrigger(index);
    return 1;
}

size_t syscall_getkey() {
    return getCharRaw();
}

size_t syscall_get_timer_ticks() {
    return getTicks();
}

size_t syscall_sleep(uint32_t millis) {
	sleep_ms(millis);
	
    return 0;
}

/**
 * @brief Инициализация системных вызовов
 * 
 * @param regs - Регистр
 * @warning If every day goes like this; How do we survive?; We are working late on the night shift; To get peace of mind!
 */
void init_syscalls(void){
	register_interrupt_handler(0x50, &syscall_handler);

	calls_table[0] = (syscall_fn_t *)syscall_env;
	calls_table[1] = (syscall_fn_t *)syscall_memory_alloc;
	calls_table[2] = (syscall_fn_t *)syscall_memory_free;
	calls_table[3] = (syscall_fn_t *)syscall_tty_write;
    calls_table[4] = (syscall_fn_t *)file_descriptor_allocate;
    calls_table[5] = (syscall_fn_t *)file_descriptor_read;
    calls_table[6] = (syscall_fn_t *)file_descriptor_close;
    calls_table[7] = (syscall_fn_t *)file_descriptor_seek;
    calls_table[8] = (syscall_fn_t *)file_descriptor_tell;
    calls_table[9] = (syscall_fn_t *)syscall_trigger_reg;
    calls_table[10] = (syscall_fn_t *)syscall_trigger_on;
    calls_table[11] = (syscall_fn_t *)syscall_trigger_off;
    calls_table[12] = (syscall_fn_t *)syscall_trigger_del;
    calls_table[13] = (syscall_fn_t *)syscall_getkey;
    calls_table[14] = (syscall_fn_t *)syscall_get_timer_ticks;
    calls_table[15] = (syscall_fn_t *)syscall_sleep;

	qemu_ok("System calls initialized!");
}
