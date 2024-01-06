#include "drv/cmos.h"

int rand_seed = 5829466;

int rand() {
	sayori_time_t time = get_time();

	rand_seed *= time.seconds;
	rand_seed -= time.minutes * time.hours + 'Z' + 'e' + 'r' + 'a';
	rand_seed *= time.hours;

	return rand_seed;
}
