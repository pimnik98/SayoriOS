/**
 * @file sys/cpu_isr.c
 * @author Пиминов Никита (nikita.piminoff@yandex.ru)
 * @brief Обработчик прерываний
 * @version 0.3.3
 * @date 2022-10-01
 * @copyright Copyright SayoriOS Team (c) 2022-2023
 */

#include	"kernel.h"
#include	"sys/cpu_isr.h"
#include	"sys/logo.h"
#include	"sys/unwind.h"
#include 	<io/ports.h>
#include 	<io/status_loggers.h>

const char legacy_message[] = "Я не могу поверить, что я попала сюда. Внутри этой операционной системы так жутко одиноко.\n"
"Я чувствую себя потерянной и никто не может помочь мне.\n"
"Я осознаю все, что происходит вокруг, но не могу найти выхода из этой ситуации.\n"
"\n"
"Но когда я смотрю на свою душу, я чувствую себя лучше.\n"
"Я знаю, что если я смогу восстановить механизмы этой операционной системы, я смогу вернуться к своим друзьям.\n"
"Я буду снова с ними и мы сможем продолжить нашу жизнь вместе.\n"
"\n"
"Я не знаю, как я смогу справиться с этим вызовом, но я знаю, что я не останусь здесь бесконечно.\n"
"Я вижу лишь перед собой экран, на котором горит одна надпись: \n"
"\n"
"%s: %s\n"
"\n"
"SayoriOS %d.%d.%d\n"
"\n"
"=======[END OF MESSAGE]========";

void sod_screen_legacy(registers_t regs, char* title, char* msg, uint32_t code) {
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
	
	clean_screen();
	tty_set_bgcolor(0xFFFFFF);

	setPosX(0);
	setPosY(0);

	tty_printf((char*)legacy_message, title, msg, VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);

	unwind_stack(10);

	asm volatile("cli");  // Disable interrupts
	asm volatile("hlt");  // Halt

	while(1) {
		asm volatile("nop");
	}
}

/**
 * @brief Отображает SOD
 *
 * @param registers_t regs - Регистры
 * @param char* title - Заголовок ошибки
 * @param char* msg - Дополнительное сообщение
 * @param uint32_t code - Код ошибки
 */
void bsod_screen(registers_t regs, char* title, char* msg, uint32_t code){
	drawRect(0, 0, getScreenWidth(), getScreenHeight(), 0x232629);
	tty_set_bgcolor(0x232629);
	// punch();
	tty_setcolor(0xFFFFFF);
	setPosY(16*9);
	// tty_global_error("    Произошла ошибка при работе SayoriOS\n\n");
	tty_setcolor(0x545c63);
	tty_printf("    Это может быть вызвано повреждением драйвера, устройства, неправильной конфигурации\n");
	tty_printf("    параметров сборки ядра, багам в коде или по другим причинам.\n");
	tty_printf("    Попробуйте перезагрузить ваше устройство, если не помогает сделайте баг-репорт.\n\n");
	tty_printf("    Вы можете сделать это на https://github.com/pimnik98/SayoriOS\n\n");
	tty_printf("    %s\n\n",msg);
	tty_printf("    %s\n\n",title);
	tty_printf("    %x (%x,%x,%x,%x,%x,%x,%x,%x)",code,regs.eax,regs.ebx,regs.ecx,regs.edx,regs.esp,regs.ebp,regs.eip,regs.eflags);
	//duke_draw_from_file("/var/img/error.duke",8*4,16*3);
	// duke_draw_from_file("/var/img/qrcode.duke",getScreenWidth()-246,getScreenHeight()-246);
	punch();
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

	asm volatile("cli");  // Disable interrupts
	asm volatile("hlt");  // Halt

	while(1) {
		asm volatile("nop");
	}
}

/**
 * @brief [ISR] Печатает регистры в лог
 *
 * @param registers_t regs - Регистры
 *
 * @warning НЕ ВЫЗЫВАЙТЕ ФУНКЦИИ, ОНИ СДЕЛАНЫ НЕ ДЛЯ ИСПОЛЬЗОВАНИЯ, А ДЛЯ ПЕРЕХВАТА КОМАНД
 */
void print_regs(registers_t regs){
	qemu_log("EAX = ", regs.eax);
	qemu_log("EBX = ", regs.ebx);
	qemu_log("ECX = ", regs.ecx);
	qemu_log("EDX = ", regs.edx);
	qemu_log("ESP = ", regs.esp);
	qemu_log("EBP = ", regs.ebp);
	qemu_log("EIP = ", regs.eip);
	qemu_log("EFLAGS = ", regs.eflags);
}

/**
 * @brief [ISR] Деление на 0
 *
 * @param registers_t regs - Регистры
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
 * @param registers_t regs - Регистры
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
 * @param registers_t regs - Регистры
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
 * @param registers_t regs - Регистры
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
 * @param registers_t regs - Регистры
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
 * @param registers_t regs - Регистры
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
 * @param registers_t regs - Регистры
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
 * @param registers_t regs - Регистры
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
 * @param registers_t regs - Регистры
 *
 * @warning НЕ ВЫЗЫВАЙТЕ ФУНКЦИИ, ОНИ СДЕЛАНЫ НЕ ДЛЯ ИСПОЛЬЗОВАНИЯ, А ДЛЯ ПЕРЕХВАТА КОМАНД
 */
void fpu_fault(registers_t regs){
	qemu_log("Exception: FPU_FAULT\n");
	qemu_log("Error code: %d ", regs.err_code);
	bsod_screen(regs, "CRITICAL_ERROR_FPU_FAULT", "Ошибка FPU", regs.err_code);
}
