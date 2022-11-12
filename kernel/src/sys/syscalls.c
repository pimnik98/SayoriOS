#define		PORT_INPUT_BYTE			0x00
#define		PORT_OUTPUT_BYTE		0x01
#define		HELLO_WORLD_CONSOLE		0x02
/**
 * @file kernel/src/sys/syscalls.c
 * @authors Александр Гагарин aka Noverd
 * @brief Прослойка для системных вызовов
 * @version 1.0
 * @date 2022-11-12
 *
 *
 */
#include "sys/syscalls.h"
#include "io/ports.h"
#include "io/tty.h"
/*!
	\brief Структура для syscalls
	Добавляеться в список calls
*/
typedef struct {
    int id;
    uint32_t (*call)(uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5);

} SYSCALL;
/*!
	\brief Список syscalls(SYSCALL)
	Добавляеться в список calls
*/
SYSCALL calls[];
int len_syscalls = 0; // Размер массива calls
/*!
	\brief Проверка наличия сискола по id
	Если id сискола есть в массиве, возращает true, иначе false
	
*/
bool check_syscall(int id) {
    for(int i=0; i<=len_syscalls; i++) 
    {
        if (calls[i].id == id){
            return true;
        }
    }
  return false;
}
/*!
	\brief Функция добавления syscall
	\warning Данная функция не проверяет наличие сискола в массиве, используйте осторожно!
*/
uint32_t s_hello_world(uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5) {
	qemu_log("Program says: Hello, World!!!");
}

void just_add_syscall(uint32_t (*call)(uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5)) {
    SYSCALL tmp = {len_syscalls, call};
    calls[len_syscalls++] = tmp;
}
uint32_t s_outb(uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5) {
	outb(arg1, arg2);
	return 0;
}
uint32_t s_inb(uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5) {
	uint_8_t ret = inb(arg1, arg2);
	return (uint32_t)ret;
	
void reg_calls(void) {
	just_add_syscall(s_outb);
	just_add_syscall(s_inb);
	just_add_syscall(s_hello_world);

}




void syscall_init() {
    register_interrupt_handler(0x50, &syscall_handler);
    qemu_log("Syscalls enabled");
    reg_calls();
}

/*!
	\brief Handler сисколлов
	Запускает syscall
*/
void syscall_handler(struct regs *r) {
    /*
        Регистры сисфункций:
        eax - номер сисфункции и результат
        ebx - параметр-указатель 1
        edx - параметр-указатель 2
        ecx - параметр-указатель 3
        esi - параметр-указатель 4
        edi - параметр-указатель 5
    */
    uint32_t arg1 = r->ebx;
    uint32_t arg2 = r->edx;
    uint32_t arg3 = r->ecx;
    uint32_t arg4 = r->esi;
    uint32_t arg5 = r->edi;
    
    for (i=0; len_syscalls>i; i++) {
        if (r->eax == calls[i].id) {
            r->eax = (uint32_t)calls[i].call(arg1, arg2, arg3, arg4, arg5);
            return void;
        }
    }

   
    qemu_log("Invalid syscall #%d", r->eax);
    tty_printf("Invalid syscall #%d", r->eax);
    }
}
