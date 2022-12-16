/**
 * @file sys/timer.c
 * @author Пиминов Никита (nikita.piminoff@yandex.ru)
 * @brief Модуль системного таймера
 * @version 0.3.0
 * @date 2022-10-01
 * @copyright Copyright SayoriOS Team (c) 2022
 */
#include	"sys/timer.h"
#include	"sys/isr.h"
#include	"drv/text_framebuffer.h"
#include	"sys/scheduler.h"

uint64_t tick = 0;			///< Количество тиков
//uint8_t hour = 0;			///< Часы
//uint8_t min = 0;			///< Минуты
//uint8_t sec = 0;			///< Секунды
uint32_t frequency = 0;		///< Частота
float uptime = 0;

/**
 * @brief Получить количество тиков
 *
 * @return uint64_t - Количество тиков с момента старта
 */
uint64_t getTicks(){
	return tick;
}

float getUptime() {
	return uptime;
}

/**
 * @brief Получить частоту таймера
 *
 * @return uint32_t - Частота таймера
 */
uint64_t getFrequency(){
	return frequency;
}

/**
 * @brief Таймер Callback
 * 
 * @param registers_t regs - Регистр
 */
static void timer_callback(registers_t regs){
	tick++;

	if(fpu_isInitialized()) {
		// uptime += 1/frequency;
		float a = 1;
		a /= frequency;
		uptime += a;
	}
	
	if (is_multitask())
		task_switch();
}

/**
 * @brief Ожидание по тикам
 *
 * @param uint32_t delay - Тики
 */
void sleep_ticks(uint32_t delay){
	uint64_t current_ticks = getTicks();
	while (1){
		if (current_ticks + delay < getTicks()){
			break;
		}
	}
}

/**
 * @brief Ожидание по милисекундам
 *
 * @param uint32_t milliseconds - Милисекунды
 */
void sleep_ms(uint32_t milliseconds) {
	uint32_t needticks = milliseconds*frequency;
	sleep_ticks(needticks/1000);
}

/**
 * @brief Ожидание по секундам
 *
 * @param uint32_t _d - Секунды
 */
void sleep(uint32_t _d) {
	sleep_ms(_d);
}

/**
 * @brief Инициализация модуля системного таймера
 * 
 * @param uint32_t - Частота
 */
void init_timer(uint32_t f){
	frequency = f;
	uint32_t divisor;
	uint8_t low;
	uint8_t high;
	register_interrupt_handler(IRQ0, &timer_callback);
	
	divisor = 1193180/frequency;
	 
	outb(0x43, 0x36);
	
	low = (uint8_t) (divisor & 0xFF);
	high = (uint8_t) ( (divisor >> 8) & 0xFF);
	
	outb(0x40, low);
	outb(0x40, high);	
}
