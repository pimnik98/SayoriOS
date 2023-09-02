#include "lib/math.h"

double acos(double x) {
	if(x < -1.0 || x > 1.0) {
		// return NAN;
		return 0.0;
	}

	return PI / 2.0 - asin(x);
}