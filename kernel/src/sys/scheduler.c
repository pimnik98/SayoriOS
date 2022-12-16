/**
 * @file sys/scheduler.c
 * @author Пиминов Никита (nikita.piminoff@yandex.ru)
 * @brief Менеджер задач
 * @version 0.3.0
 * @date 2022-10-01
 * @copyright Copyright SayoriOS Team (c) 2022
 */
#include	"sys/scheduler.h"
#include	"lib/string.h"
#include	"io/ports.h"

uint32_t next_pid = 0;			///< Следующий ID задачи (PID)
uint32_t next_thread_id = 0;	///< Следующий ID потока
list_t process_list;			///< Список процессов
list_t thread_list;				///< Список потоков
bool multi_task = false;		///< Готова ли система к многозадачности
process_t* kernel_proc = 0;		///< Обработчик процесса ядра
thread_t* kernel_thread = 0;	///< Обработчик основного потока ядра
process_t* current_proc;		///< Текущий процесс
thread_t* current_thread;		///< Текущий поток
extern uint32_t init_esp;

/**
 * @brief Инициализация менеджера задач
 */
void init_task_manager(void){
	uint32_t esp = 0;
	asm volatile("mov %%esp, %0":"=a"(esp));
	
	/* Disable all interrupts */
	asm volatile ("cli");

	list_init(&process_list);
	list_init(&thread_list);

	/* Create kernel process */
	kernel_proc = (process_t*) kmalloc(sizeof(process_t));

	memset(kernel_proc, 0, sizeof(process_t));

	kernel_proc->pid = next_pid++;
	kernel_proc->page_dir = get_kernel_dir();
	kernel_proc->list_item.list = NULL;
	kernel_proc->threads_count = 1;
	strcpy(kernel_proc->name, "Kernel");
	kernel_proc->suspend = false;

	list_add(&process_list, &kernel_proc->list_item);

	/* Create kernel thread */
	kernel_thread = (thread_t*) kmalloc(sizeof(thread_t));

	memset(kernel_thread, 0, sizeof(thread_t));

	kernel_thread->process = kernel_proc;
	kernel_thread->list_item.list = NULL;
	kernel_thread->id = next_thread_id++;
	kernel_thread->stack_size = 0x4000;
	kernel_thread->suspend = false;
	kernel_thread->esp = esp;
	kernel_thread->stack_top = init_esp;

	list_add(&thread_list, &kernel_thread->list_item);

	current_proc = kernel_proc;
	current_thread = kernel_thread;	

	/* Enable multitasking flag */
	multi_task = true;

	asm volatile ("sti");
}

/**
 * @brief Переключение задач
 */
void switch_task(void){
	if (multi_task){
		/* Disable all interrupts */
		asm volatile ("pushf; cli");

		/* Remember current thread state */
		asm volatile ("mov %%esp, %0":"=a"(current_thread->esp));

		current_thread = (thread_t*) current_thread->list_item.next;		

		/* Set current page directory */
		asm volatile ("mov %0, %%cr3"::"a"(current_proc->page_dir));
		/* Set stack */
		asm volatile ("mov %0, %%esp"::"a"(current_thread->esp));

		/* Enable interrupts */
		asm volatile ("popf");
	}
}

 /**
 * @brief Получить текущий обработчик процесса
 *
 * @return process_t* - Текущий обработчик задачи
 */
process_t* get_current_proc(void)
{
	return current_proc;
}

/**
 * @brief Создание потока
 * 
 * @param process_t* proc - Процесс
 * @param void* entry_point - Точка входа
 * @param size_t stack_size - Размер стека
 * @param bool kernel - Функция ядра?
 * @param bool suspend - Остановлено?
 *
 * @return thread_t* - Поток
 */
thread_t* thread_create(process_t* proc,void* entry_point,size_t stack_size,bool kernel,bool suspend){
	void*	stack = NULL;
	uint32_t	eflags;

	/* Disable all interrupts */
	asm volatile ("cli");

	/* Create new thread handler */
	thread_t* tmp_thread = (thread_t*) kmalloc(sizeof(thread_t));

	/* Clear memory */
	memset(tmp_thread, 0, sizeof(thread_t));

	/* Initialization of thread  */
	tmp_thread->id = next_thread_id++;
	tmp_thread->list_item.list = NULL;
	tmp_thread->process = proc;
	tmp_thread->stack_size = stack_size;
	tmp_thread->suspend = suspend;/* */
	tmp_thread->entry_point = (uint32_t) entry_point;

	/* Create thread's stack */
	stack = (void*) kmalloc(stack_size);

	tmp_thread->stack = stack;
	tmp_thread->esp = (uint32_t) stack + stack_size - 12;
	tmp_thread->stack_top = (uint32_t) stack + stack_size;

	/* Add thread to ring queue */
	list_add(&thread_list, &tmp_thread->list_item);

	/* Thread's count increment */
	proc->threads_count++;

	/* Fill stack */

	/* Create pointer to stack frame */
	uint32_t* esp = (uint32_t*) (stack + stack_size);

	asm volatile ("pushf; pop %0":"=r"(eflags));

	eflags |= (1 << 9);

	esp[-1] = (uint32_t) entry_point;
	esp[-3] = eflags;

	/* Enable all interrupts */
	asm volatile ("sti");

	return tmp_thread;
}

/**
 * @brief Остановить поток
 * 
 * @param thread_t* thread - Поток
 * @param bool suspend - Вкл/выкл
 */
void thread_suspend(thread_t* thread, bool suspend){
	thread->suspend = suspend;
}

/**
 * @brief Завершить текущий поток
 * 
 * @param thread_t* thread - Поток
 */
void thread_exit(thread_t* thread){
	/* Disable all interrupts */
	asm volatile ("cli");

	/* Remove thread from queue */
	list_remove(&thread->list_item);
	
	thread->process->threads_count--;

	/* Free thread's memory (handler and stack) */
	kfree(thread->stack);
	kfree(thread);

	/* Load to ECX switch function address */
	asm volatile ("mov %0, %%ecx"::"a"(&task_switch));

	/* Enable all interrupts */
	asm volatile ("sti");

	/* Jump to switch_task() */
	asm volatile ("call *%ecx");
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
 * @param void* entry_point - Точка входа
 * @param size_t stack_size - Размер стека
 */
void init_user_mode(void* entry_point, size_t stack_size){
    void* user_stack = (void*) kmalloc(stack_size);
    user_mode_switch(entry_point, (uint32_t) user_stack + stack_size);
}
