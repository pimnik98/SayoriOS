/**
 * @file sys/cpu_isr.c
 * @author Пиминов Никита (nikita.piminoff@yandex.ru)
 * @brief Обработчик прерываний
 * @version 0.3.4
 * @date 2022-10-01
 * @copyright Copyright SayoriOS Team (c) 2022-2023
 */

#include	"sys/cpu_isr.h"
#include	"sys/logo.h"
#include	"sys/unwind.h"
#include 	<io/ports.h>
#include 	<io/status_loggers.h>

void sod_screen_legacy(registers_t regs, char* title, char* msg, uint32_t code) {
	qemu_err("=== ЯДРО УПАЛО =======================================\n");
	qemu_err("| ");
	qemu_err("| Наименование: %s",title);
	qemu_err("| Код ошибки: %x",code);
	qemu_err("| Сообщение: %s",msg);
	qemu_err("| EAX: %x",regs.eax);
	qemu_err("| EBX: %x",regs.ebx);
	qemu_err("| ECX: %x",regs.ecx);
	qemu_err("| EDX: %x",regs.edx);
	qemu_err("| ESP: %x",regs.esp);
	qemu_err("| EBP: %x",regs.ebp);
	qemu_err("| EIP: %x",regs.eip);
	qemu_err("| EFLAGS: %x",regs.eflags);
	qemu_err("| ");
	qemu_err("======================================================\n");

	// extern char _temp_funcname[1024];
	//
	// bool exists = get_func_name_by_addr(regs.eip);
	//
	// qemu_err("Failed on: %s", exists ? _temp_funcname : "???");

//	unwind_stack(10);
//
//    heap_dump();

    __asm__ volatile("cli");  // Disable interrupts
	__asm__ volatile("hlt");  // Halt

	while(1) {
		__asm__ volatile("nop");
	}
}

/**
 * @brief Отображает SOD
 *
 * @param regs - Регистры
 * @param title - Заголовок ошибки
 * @param msg - Дополнительное сообщение
 * @param code - Код ошибки
 */
void bsod_screen(registers_t regs, char* title, char* msg, uint32_t code){
	qemu_log("=== ЯДРО УПАЛО =======================================\n");
	qemu_log("| ");
	qemu_log("| Наименование: %s",title);
	qemu_log("| Код ошибки: %x",code);
	qemu_log("| Сообщение: %s",msg);
	qemu_log("| EAX: %x",regs.eax);
	qemu_log("| EBX: %x",regs.ebx);
	qemu_log("| ECX: %x",regs.ecx);
	qemu_log("| EDX: %x",regs.edx);
	qemu_log("| ESP: %x",regs.esp);
	qemu_log("| EBP: %x",regs.ebp);
	qemu_log("| EIP: %x",regs.eip);
	qemu_log("| EFLAGS: %x",regs.eflags);
	qemu_log("| ");
	qemu_log("======================================================\n");

	unwind_stack(10);

	__asm__ volatile("cli");  // Disable interrupts
	__asm__ volatile("hlt");  // Halt

	while(1) {
		__asm__ volatile("nop");
	}
}

/**
 * @brief [ISR] Печатает регистры в лог
 *
 * @param regs - Регистры
 *
 * @warning НЕ ВЫЗЫВАЙТЕ ФУНКЦИИ, ОНИ СДЕЛАНЫ НЕ ДЛЯ ИСПОЛЬЗОВАНИЯ, А ДЛЯ ПЕРЕХВАТА КОМАНД
 */
void print_regs(registers_t regs){
	qemu_log("EAX = %x", regs.eax);
	qemu_log("EBX = %x", regs.ebx);
	qemu_log("ECX = %x", regs.ecx);
	qemu_log("EDX = %x", regs.edx);
	qemu_log("ESP = %x", regs.esp);
	qemu_log("EBP = %x", regs.ebp);
	qemu_log("EIP = %x", regs.eip);
	qemu_log("EFLAGS = %x", regs.eflags);
}

/**
 * @brief [ISR] Деление на 0
 *
 * @param regs - Регистры
 *
 * @warning НЕ ВЫЗЫВАЙТЕ ФУНКЦИИ, ОНИ СДЕЛАНЫ НЕ ДЛЯ ИСПОЛЬЗОВАНИЯ, А ДЛЯ ПЕРЕХВАТА КОМАНД
 */
void division_by_zero(registers_t regs)
{
	qemu_log("Exception: DIVISION BY ZERO\n");
	print_regs(regs);
	bsod_screen(regs,"CRITICAL_ERROR_DZ_DIVISION_BY_ZERO","Деление на ноль",regs.eax);
}

/**
 * @brief [ISR] Невалидный код
 *
 * @param regs - Регистры
 *
 * @warning НЕ ВЫЗЫВАЙТЕ ФУНКЦИИ, ОНИ СДЕЛАНЫ НЕ ДЛЯ ИСПОЛЬЗОВАНИЯ, А ДЛЯ ПЕРЕХВАТА КОМАНД
 */
void fault_opcode(registers_t regs){
	qemu_log("FAULT OPERATION CODE...\n");
	print_regs(regs);
	bsod_screen(regs,"CRITICAL_ERROR_UD_FAULT_OPERATION_CODE","Невалидный код",regs.eax);
}

/**
 * @brief [ISR] Двойное исключение
 *
 * @param regs - Регистры
 *
 * @warning НЕ ВЫЗЫВАЙТЕ ФУНКЦИИ, ОНИ СДЕЛАНЫ НЕ ДЛЯ ИСПОЛЬЗОВАНИЯ, А ДЛЯ ПЕРЕХВАТА КОМАНД
 */
void double_error(registers_t regs){
	qemu_log("Exception: DOUBLE EXCEPTION\n");
	qemu_log("Error code: %d", regs.err_code);
	bsod_screen(regs,"CRITICAL_ERROR_DF_DOUBLE_EXCEPTION","Двойное исключение",regs.err_code);
}

/**
 * @brief [ISR] Недействительный TSS
 *
 * @param regs - Регистры
 *
 * @warning НЕ ВЫЗЫВАЙТЕ ФУНКЦИИ, ОНИ СДЕЛАНЫ НЕ ДЛЯ ИСПОЛЬЗОВАНИЯ, А ДЛЯ ПЕРЕХВАТА КОМАНД
 */
void invalid_tss(registers_t regs){
	uint32_t ext = regs.err_code & EXT_BIT;
	uint32_t idt = regs.err_code & IDT_BIT;
	uint32_t ti = regs.err_code & TI_BIT;
	uint32_t selector = regs.err_code & ERR_CODE_MASK;

	qemu_log("Exception: INVALID TSS\n");
	qemu_log("cause of error: ");

	char* msg = "Недействительный TSS";
	if (ext)
	{
		qemu_log("HARDWARE INTERRUPT\n");
		msg = "Аппаратное прерывание";
	}

	if (idt)
	{
		qemu_log("IDT GATE\n");
		msg = "Шлюз IDT";
	}

	if (ti)
	{
		qemu_log("LDT GATE\n");
		msg = "Шлюз LDT";
	}

	qemu_log("Invalid selector: %d", selector);
	bsod_screen(regs, "CRITICAL_ERROR_TS_INVALID_TS", msg, selector);
}

/**
 * @brief [ISR] Недействительный сегмент
 *
 * @param regs - Регистры
 *
 * @warning НЕ ВЫЗЫВАЙТЕ ФУНКЦИИ, ОНИ СДЕЛАНЫ НЕ ДЛЯ ИСПОЛЬЗОВАНИЯ, А ДЛЯ ПЕРЕХВАТА КОМАНД
 */
void segment_is_not_available(registers_t regs){
	uint32_t ext = regs.err_code & EXT_BIT;
	uint32_t idt = regs.err_code & IDT_BIT;
	uint32_t ti = regs.err_code & TI_BIT;
	uint32_t selector = regs.err_code & ERR_CODE_MASK;

	qemu_log("Exception: SEGMENT IS'T AVAILABLE\n");
	qemu_log("cause of error: ");

	char* msg = "СЕГМЕНТ НЕДОСТУПЕН";
	if (ext)
	{
		qemu_log("HARDWARE INTERRUPT\n");
		msg = "Аппаратное прерывание";
	}

	if (idt)
	{
		qemu_log("IDT GATE\n");
		msg = "Шлюз IDT";
	}

	if (ti)
	{
		qemu_log("LDT GATE\n");
		msg = "Шлюз LDT";
	}

	qemu_log("Invalid selector: %d", selector);
	bsod_screen(regs,"CRITICAL_ERROR_NP_SEGMENT_IST_AVAILABLE", msg, selector);
}

/**
 * @brief [ISR] Ошибка стека
 *
 * @param regs - Регистры
 *
 * @warning НЕ ВЫЗЫВАЙТЕ ФУНКЦИИ, ОНИ СДЕЛАНЫ НЕ ДЛЯ ИСПОЛЬЗОВАНИЯ, А ДЛЯ ПЕРЕХВАТА КОМАНД
 */
void stack_error(registers_t regs){
	qemu_log("Exception: STACK ERROR\n");
	qemu_log("Error code: %d ", regs.err_code);
	bsod_screen(regs,"CRITICAL_ERROR_SS_STACK_ERROR","Ошибка стека",regs.err_code);
}

/**
 * @brief [ISR] Общая ошибка защиты
 *
 * @param regs - Регистры
 *
 * @warning НЕ ВЫЗЫВАЙТЕ ФУНКЦИИ, ОНИ СДЕЛАНЫ НЕ ДЛЯ ИСПОЛЬЗОВАНИЯ, А ДЛЯ ПЕРЕХВАТА КОМАНД
 */
void general_protection_error(registers_t regs) {
	qemu_log("Exception: GENERAL PROTECTION ERROR\n");
	qemu_log("Error code: %d", regs.err_code);

	bsod_screen(regs,"CRITICAL_ERROR_GP_GENERAL_PROTECTION", "Общая ошибка защиты", regs.err_code);
}

/**
 * @brief [ISR] Переполнение памяти буфера
 *
 * @param regs - Регистры
 *
 * @warning НЕ ВЫЗЫВАЙТЕ ФУНКЦИИ, ОНИ СДЕЛАНЫ НЕ ДЛЯ ИСПОЛЬЗОВАНИЯ, А ДЛЯ ПЕРЕХВАТА КОМАНД
 */
void page_fault(registers_t regs){
	uint32_t fault_addr = read_cr2();
	int present = !(regs.err_code & 0x1);		/* Page not present */
	uint32_t rw = regs.err_code & 0x2;				/* Page is read only */
	uint32_t user = regs.err_code & 0x4;				/* User mode */
	uint32_t reserved = regs.err_code & 0x8;			/* Reserved bits is wrote */
	uint32_t id = regs.err_code & 0x10;				/* Instruction fetch */
	qemu_log("Page fault: ");
	char* msg = "Переполнение памяти буфера";
	if (present){
		qemu_log("NOT PRESENT, ");
		msg = "Память по данному адресу недоступна";
	}
	if (rw){
		qemu_log("READ ONLY, ");
		msg = "Память только для чтения";
	}
	if (user){
		qemu_log("USER MODE,  ");
		msg = "Память только для пользователя";
	}
	if (reserved){
		qemu_log("WRITING TO RESERVED BITS, ");
		msg = "Попытка записи в зарез. биты";
	}
	if (id){
		qemu_log("EIP error ");
		msg = "Ошибка EIP";
	}
	qemu_log("at address (virtual) %x",fault_addr);

	// Prevent kmallocs (in bsod_screen()) in Page Fault
	sod_screen_legacy(regs, "CRITICAL_ERROR_PF_PAGE_FAULT", msg, fault_addr);
}

/**
 * @brief [ISR] Ошибка FPU
 *
 * @param regs - Регистры
 *
 * @warning НЕ ВЫЗЫВАЙТЕ ФУНКЦИИ, ОНИ СДЕЛАНЫ НЕ ДЛЯ ИСПОЛЬЗОВАНИЯ, А ДЛЯ ПЕРЕХВАТА КОМАНД
 */
void fpu_fault(registers_t regs){
	qemu_log("Exception: FPU_FAULT\n");
	qemu_log("Error code: %d ", regs.err_code);
	bsod_screen(regs, "CRITICAL_ERROR_FPU_FAULT", "Ошибка FPU", regs.err_code);
}
