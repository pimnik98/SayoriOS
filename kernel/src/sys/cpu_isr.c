/**
 * @file sys/bootscreen.c
 * @author Пиминов Никита (nikita.piminoff@yandex.ru)
 * @brief Обработчик прерываний
 * @version 0.3.0
 * @date 2022-10-01
 * @copyright Copyright SayoriOS Team (c) 2022
 */
#include	"kernel.h"
#include	"sys/cpu_isr.h"
#include 	<io/ports.h>

/**
 * @brief Отображает BSOD
 *
 * @param registers_t regs - Регистры
 * @param char* title - Заголовок ошибки
 * @param char* msg - Дополнительное сообщение
 * @param uint32_t code - Код ошибки
 */
void bsod_screen(registers_t regs,char* title, char* msg,uint32_t code){
	drawRect(0,0,getWidthScreen(),getHeightScreen(),0xA5383B);
	tty_set_bgcolor(0xA5383B);
	punch();
	tty_setcolor(0xFFD2D3);
	setPosX(0);
	setPosY(0);
	drawASCIILogo(1);
	tty_printf("=== КРИТИЧЕСКАЯ ОШИБКА ===============================\n");
	tty_printf("| \n");
	tty_printf("| - Произошла ошибка при работе ядра\n");
	tty_printf("| \n");
	tty_printf("=== ЧТО МОЖНО СДЕЛАТЬ? ===============================\n");
	tty_printf("| \n");
	tty_printf("| 1. Повторить заного действия, если ошибка\n");
	tty_printf("|    не исчезает, надо сделать баг-репорт\n");
	tty_printf("| \n");
	tty_printf("| 2. Для этого посетите сайт или репозиторий\n");
	tty_printf("|    проекта, сделать можно по этим ссылкам:\n");
	tty_printf("|     * Github: github.com/pimnik98/SayoriOS\n");
	tty_printf("|     * VK: vk.com/sayorios\n");
	tty_printf("|     * Сайт: SayoriOS.Piminoff.Ru\n");
	tty_printf("| \n");
	tty_printf("| 3. Для GitHub'a:\n");
	tty_printf("|     * Создать Issues\n");
	tty_printf("|    Для остальных сайтах, следовать руководству\n");
	tty_printf("| \n");
	tty_printf("=== ПОДРОБНЕЕ ОБ ОШИБКЕ ==============================\n");
	tty_printf("| \n");
	tty_printf("| Наименование: %s\n",title);
	tty_printf("| Код ошибки: %x\n",code);
	tty_printf("| Сообщение: %s\n",msg);
	tty_printf("| EAX: %x\n",regs.eax);
	tty_printf("| EBX: %x\n",regs.ebx);
	tty_printf("| ECX: %x\n",regs.ecx);
	tty_printf("| EDX: %x\n",regs.edx);
	tty_printf("| ESP: %x\n",regs.esp);
	tty_printf("| EBP: %x\n",regs.ebp);
	tty_printf("| EIP: %x\n",regs.eip);
	tty_printf("| EFLAGS: %x\n",regs.eflags);
	tty_printf("| Более подробная информация была записана в лог файл!\n");
	tty_printf("| \n");
	tty_printf("======================================================\n");

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
	bsod_screen(regs,"#DE - Деление на ноль","нет",regs.eax);
	print_regs(regs);
	while (1);
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
	bsod_screen(regs,"#UD - Невалидный код","нет",regs.eax);

	while (1);
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
	bsod_screen(regs,"#DF - Двойное исключение","нет",regs.err_code);

	while (1);
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

	char* msg = "нет";
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
	bsod_screen(regs,"#TS - Недействительный TSS","msg",selector);

	while (1);
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

	char* msg = "нет";
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
	bsod_screen(regs,"#NP - СЕГМЕНТ НЕДОСТУПЕН","msg",selector);

	while (1);
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
	bsod_screen(regs,"#SS - Ошибка стека","нет",regs.err_code);

	while (1);
}

/**
 * @brief [ISR] Общая ошибка защиты
 *
 * @param registers_t regs - Регистры
 *
 * @warning НЕ ВЫЗЫВАЙТЕ ФУНКЦИИ, ОНИ СДЕЛАНЫ НЕ ДЛЯ ИСПОЛЬЗОВАНИЯ, А ДЛЯ ПЕРЕХВАТА КОМАНД
 */
void general_protection_error(registers_t regs){
	qemu_log("Exception: GENERAL PROTECTION ERROR\n");
	qemu_log("Error code: %d", regs.err_code);

	bsod_screen(regs,"#GP - Общая ошибка защиты", "нет", regs.err_code);
	while (1);
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
	int rw = regs.err_code & 0x2;				/* Page is read only */
	int user = regs.err_code & 0x4;			/* User mode */
	int reserved = regs.err_code & 0x8;		/* Reserved bits is writed */
	int id = regs.err_code & 0x10;			/* */
	qemu_log("Page fault: ");
	char* msg = "нет";
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
	qemu_log("at address %x",fault_addr);

	bsod_screen(regs,"#PF - Переполнение памяти буфера",msg,fault_addr);
	while (1);
}
