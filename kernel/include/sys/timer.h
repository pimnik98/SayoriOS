#ifndef		TIMER_H
#define		TIMER_H

#define		BASE_FREQ	1000

#include	"common.h"

#define sleep(_d) sleep_ms((_d) * 1000);
#define timestamp() (getTicks() / (getFrequency() / 1000))

size_t getTicks();
double getUptime();
size_t getFrequency();
void sleep_ticks(uint32_t delay);
void sleep_ms(uint32_t milliseconds);


void init_timer(uint32_t f);


#endif
