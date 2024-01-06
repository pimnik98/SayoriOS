/**
 * @file sys/scheduler.c
 * @author Пиминов Никита (nikita.piminoff@yandex.ru)
 * @brief Менеджер задач
 * @version 0.3.4
 * @date 2022-10-01
 * @copyright Copyright SayoriOS Team (c) 2022-2023
 */
#include	"sys/scheduler.h"
#include	"lib/string.h"
#include	"io/ports.h"
#include "mem/vmm.h"

list_t process_list;			///< Список процессов
list_t thread_list;				///< Список потоков

uint32_t next_pid = 0;			///< Следующий ID задачи (PID)
uint32_t next_thread_id = 0;	///< Следующий ID потока
bool multi_task = false;		///< Готова ли система к многозадачности
process_t* kernel_proc = 0;		///< Обработчик процесса ядра
thread_t* kernel_thread = 0;	///< Обработчик основного потока ядра
process_t* current_proc = 0;	///< Текущий процесс
thread_t* current_thread = 0;	///< Текущий поток
extern uint32_t init_esp;

bool scheduler_working = true;

extern physical_addr_t kernel_page_directory;

/**
 * @brief Инициализация менеджера задач
 */
void init_task_manager(void){
	uint32_t esp = 0;
	__asm__ volatile("mov %%esp, %0" : "=a"(esp));

	/* Disable all interrupts */
	__asm__ volatile ("cli");

	list_init(&process_list);
	list_init(&thread_list);

	/* Create kernel process */
	kernel_proc = (process_t*)kmalloc(sizeof(process_t));

	memset(kernel_proc, 0, sizeof(process_t));


	kernel_proc->pid = next_pid++;
    // NOTE: Page directory address must be PHYSICAL!
	kernel_proc->page_dir = kernel_page_directory;
	kernel_proc->list_item.list = nullptr;
	kernel_proc->threads_count = 1;
	strcpy(kernel_proc->name, "Kernel");
	kernel_proc->suspend = false;

	list_add(&process_list, &kernel_proc->list_item);

	/* Create kernel thread */
	kernel_thread = (thread_t*) kmalloc(sizeof(thread_t));

	memset(kernel_thread, 0, sizeof(thread_t));

	kernel_thread->process = kernel_proc;
	kernel_thread->list_item.list = nullptr;
	kernel_thread->id = next_thread_id++;
	kernel_thread->stack_size = 0x4000;
	kernel_thread->suspend = false;
	kernel_thread->esp = esp;
	kernel_thread->stack_top = init_esp;

	list_add(&thread_list, &kernel_thread->list_item);

	current_proc = kernel_proc;
	current_thread = kernel_thread;

	__asm__ volatile ("sti");

	/* Enable multitasking flag */
	multi_task = true;

    qemu_ok("OK");
}

void scheduler_mode(bool on) {
	scheduler_working = on;
}

void create_process(void* entry_point, char* name, bool suspend, bool is_kernel) {
	__asm__ volatile("cli");

    process_t* proc = (process_t*)kcalloc(1, sizeof(process_t));

    void* virt = clone_kernel_page_directory();
    uint32_t phys = virt2phys(get_kernel_page_directory(), (virtual_addr_t) virt);

    qemu_note("New page directory at: V%x => P%x", virt, phys);

	proc->pid = next_pid++;
//	proc->page_dir = kernel_page_directory;
	proc->page_dir = phys;
	proc->list_item.list = nullptr;  // No nested processes hehe :)
	proc->threads_count = 1;
	strcpy(proc->name, name);
	proc->suspend = suspend;

	list_add(&process_list, &proc->list_item);

	thread_create(proc, entry_point, DEFAULT_STACK_SIZE, is_kernel, suspend);
	list_add(&thread_list, &kernel_thread->list_item);

	__asm__ volatile("sti");
}

 /**
 * @brief Получить текущий обработчик процесса
 *
 * @return process_t* - Текущий обработчик задачи
 */
process_t* get_current_proc(void) {
	return current_proc;
}

/**
 * @brief Создание потока
 * 
 * @param proc - Процесс
 * @param entry_point - Точка входа
 * @param stack_size - Размер стека
 * @param kernel - Функция ядра?
 * @param suspend - Остановлено?
 *
 * @return thread_t* - Поток
 */
thread_t* thread_create(process_t* proc, void* entry_point, size_t stack_size,
						bool kernel, bool suspend){
	void*	stack = nullptr;
	uint32_t	eflags;

	/* Disable all interrupts */
	__asm__ volatile ("cli");

	/* Create new thread handler */
	thread_t* tmp_thread = (thread_t*) kmalloc(sizeof(thread_t));

	/* Clear memory */
	memset(tmp_thread, 0, sizeof(thread_t));

	/* Initialization of thread  */
	tmp_thread->id = next_thread_id++;
	tmp_thread->list_item.list = nullptr;
	tmp_thread->process = proc;
	tmp_thread->stack_size = stack_size;
	tmp_thread->suspend = suspend;/* */
	tmp_thread->entry_point = (uint32_t) entry_point;

	/* Create thread's stack */
	stack = (void*) kcalloc(stack_size, 1);

	tmp_thread->stack = stack;
	tmp_thread->esp = (uint32_t) stack + stack_size - 12;
	tmp_thread->stack_top = (uint32_t) stack + stack_size;

	/* Add thread to ring queue */
	list_add(&thread_list, &tmp_thread->list_item);

	/* Thread's count increment */
	proc->threads_count++;

	/* Fill stack */

	/* Create pointer to stack frame */
	uint32_t* esp = (uint32_t*) ((char*)stack + stack_size);

	__asm__ volatile ("pushf; pop %0":"=r"(eflags));

	eflags |= (1 << 9);

	esp[-1] = (uint32_t) entry_point;
	esp[-3] = eflags;

	/* Enable all interrupts */
	__asm__ volatile ("sti");

	return tmp_thread;
}

/**
 * @brief Остановить поток
 * 
 * @param thread - Поток
 * @param suspend - Вкл/выкл
 */
void thread_suspend(thread_t* thread, bool suspend){
	thread->suspend = suspend;
}

/**
 * @brief Завершить текущий поток
 * 
 * @param thread - Поток
 */
void thread_exit(thread_t* thread){
	/* Disable all interrupts */
	__asm__ volatile ("cli");

	/* Remove thread from queue */
	list_remove(&thread->list_item);
	
	thread->process->threads_count--;

	/* Free thread's memory (handler and stack) */
	kfree(thread->stack);
	kfree(thread);

	/* Load to ECX switch function address */
	__asm__ volatile ("mov %0, %%ecx"::"a"(&task_switch));

	/* Enable all interrupts */
	__asm__ volatile ("sti");

	/* Jump to switch_task() */
	__asm__ volatile ("call *%ecx");
}

/**
 * @brief Получение состояния о мультипотоке
 *
 * @return bool - true - если готово к работе
 */
bool is_multitask(void){
    return multi_task;
}

/**
 * @brief Переключиться в пользовательский режим
 * 
 * @param entry_point - Точка входа
 * @param stack_size - Размер стека
 */
void init_user_mode(void* entry_point, size_t stack_size){
    void* user_stack = (void*) kmalloc(stack_size);
    user_mode_switch(entry_point, (uint32_t) user_stack + stack_size);
}
