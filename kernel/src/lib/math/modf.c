#include "lib/math.h"

double modf(double value, double* intPart) {
    if (intPart != NULL) {
        *intPart = (value >= 0) ? floor(value) : ceil(value);
    }
    return value - (*intPart);
}