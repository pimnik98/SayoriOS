#include "lib/math.h"

double sin(double rads) {
	size_t steps = 16;
	double result = rads / fac(1);

	bool flag = false;

	for (size_t i = 3; i < steps + 1; i += 2) {
		if (!flag) {
			result -= pow(rads, i) / fac(i);
		} else {
			result += pow(rads, i) / fac(i);
		}

		flag = !flag;
	}

	return result;
}
