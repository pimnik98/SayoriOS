#include "lib/math.h"

double atan_ipart(double t) {
	return pow(1.0 + pow(t, 2), -1);
}

double atan(double x) {
	return trapezoidal_rule(
		atan_ipart,
		0,
		x,
		ATAN_STEPS
	);
}
