/**
 * @file sys/timer.c
 * @author Пиминов Никита (nikita.piminoff@yandex.ru)
 * @brief Модуль системного таймера
 * @version 0.3.4
 * @date 2022-10-01
 * @copyright Copyright SayoriOS Team (c) 2022-2023
 */
#include	"sys/timer.h"
#include	"sys/isr.h"
//#include	"sys/scheduler.h"
#include	"drv/fpu.h"
#include	"io/ports.h"

extern bool scheduler_working;

size_t tick = 0;			///< Количество тиков
size_t frequency = CLOCK_FREQ;		///< Частота

// FIXME: Invalid and deprecated implementation.
void microseconds_delay(size_t microseconds) {
    for (size_t i = 0; i < microseconds; ++i)
        inb(0x80);
}

/**
 * @brief Получить количество тиков
 *
 * @return size_t - Количество тиков с момента старта
 */
size_t getTicks(){
	return tick;
}

double getUptime() {
	if(getFrequency() == 0) {
		return 0.0;
	}else{
		return (double)tick / (double)frequency;
	}
}

/**
 * @brief Получить частоту таймера
 *
 * @return uint32_t - Частота таймера
 */
size_t getFrequency(){
	return frequency;
}

/**
 * @brief Таймер Callback
 * 
 * @param regs - Регистр
 */
static void timer_callback(__attribute__((unused)) registers_t regs){
	tick++;
	
	if (is_multitask() && scheduler_working)
		task_switch();
}

/**
 * @brief Ожидание по тикам
 *
 * @param delay - Тики
 */
void sleep_ticks(uint32_t delay){
	size_t current_ticks = getTicks();
	while (1){
		if (current_ticks + delay < getTicks()){
			break;
		}
	}
}

/**
 * @brief Ожидание по милисекундам
 *
 * @param milliseconds - Милисекунды
 */
void sleep_ms(uint32_t milliseconds) {
	uint32_t needticks = milliseconds * frequency;
	sleep_ticks(needticks / 1000);

	// (milliseconds * frequency + 500) / 1000
}

/**
 * @brief Инициализация модуля системного таймера
 * 
 * @param - Частота
 */
void init_timer(uint32_t f){
	frequency = f;

	uint32_t divisor;
	uint8_t low;
	uint8_t high;

	divisor = BASE_FREQ / f;

	outb(0x43, 0x36);

	low = (uint8_t) (divisor & 0xFF);
	high = (uint8_t) ((divisor >> 8) & 0xFF);

	outb(0x40, low);
	outb(0x40, high);

	register_interrupt_handler(IRQ0, &timer_callback);
}
