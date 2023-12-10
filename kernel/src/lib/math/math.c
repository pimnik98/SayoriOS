// Math library by NDRAEY

#include "lib/math.h"

size_t fac(size_t x) {
	size_t a = 1;

	for (size_t i = 1; i < x + 1; i++) {
		a *= i;
	}
	
	return a;
}

size_t ipow(size_t val, size_t exp) {
	size_t value = val;
	for (size_t i = 1; i < exp; i++) {
		value *= val;
	}

	return value;
}

double deg2rad(double deg) {
	return deg * (PI / 180.0);
}

double rad2deg(double rad) {
	return rad * (180.0 / PI);
}