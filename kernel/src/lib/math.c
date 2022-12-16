// Simple Sine/Cosine (And other math) library by NDRAEY

#include "lib/math.h"

size_t fac(size_t x)
{
	size_t a = 1;
	for (size_t i = 1; i < x + 1; i++)
	{
		a *= i;
	}
	return a;
}

double pow(double val, size_t exp) {
	double value = val;
	for (size_t i = 1; i < exp; i++) {
		value *= val;
	}

	return value;
}

size_t ipow(size_t val, size_t exp) {
	size_t value = val;
	for (size_t i = 1; i < exp; i++) {
		value *= val;
	}

	return value;
}

double deg2rad(size_t deg) {
	return deg * (PI / 180);
}

double sin(double val)
{
	size_t steps = 16;
	double rads = deg2rad(val);
	double result = rads / fac(1);

	bool flag = false;

	for (size_t i = 3; i < steps + 1; i += 2)
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

double cos(double val)
{
	size_t steps = 16;
	double rads = deg2rad(val);
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

double tan(double val) {
	double s = sin(val);
	double c = cos(val);
	return s / c;
}
