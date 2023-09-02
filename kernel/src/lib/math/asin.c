#include "lib/math.h"

double asin_ipart(double t) {
	return pow(1.0 - pow(t, 2.0), -0.5);
}

double asin(double x) {
	if(x < -1.0 || x > 1.0) {
		return NAN;
	}

	if(x == 1.0) {
		return PI / 2;
	}

	return trapezoidal_rule(
		asin_ipart,
		0,
		x,
		ASIN_STEPS
	);
}
