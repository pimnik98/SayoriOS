#ifndef		TIMER_H
#define		TIMER_H

#include	"common.h"

#define		BASE_FREQ	1000

void init_timer(uint32_t frequency);
float getUptime();

#endif
