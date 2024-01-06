#pragma once

#define		BASE_FREQ	1193180
#define     CLOCK_FREQ  1000

#include	"common.h"

#define sleep(_d) sleep_ms((_d) * CLOCK_FREQ);
#define timestamp() (getTicks() / (getFrequency() / 1000))

size_t getTicks();
double getUptime();
size_t getFrequency();
void sleep_ticks(uint32_t delay);
void sleep_ms(uint32_t milliseconds);
void init_timer(uint32_t f);
