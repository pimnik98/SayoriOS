#include "drv/cmos.h"

int rand_seed = 5829466;

int rand() {
	rand_seed = (164525 * rand_seed + 101390423) % 4294967296;
	return rand_seed;
}
