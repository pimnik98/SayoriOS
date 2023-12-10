#include "lib/math.h"

double tan(double rads) {
	double s = sin(rads);
	double c = cos(rads);

	return s / c;
}
