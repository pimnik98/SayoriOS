#include "lib/math.h"

double cos(double rads) {
	size_t steps = 16;
	double result = 1.0;

	bool flag = false;

	for (size_t i = 2; i < steps + 1; i += 2)
	{
		if (!flag)
		{
			result -= pow(rads, i) / fac(i);
		}
		else
		{
			result += pow(rads, i) / fac(i);
		}
		flag = !flag;
	}

	return result;
}