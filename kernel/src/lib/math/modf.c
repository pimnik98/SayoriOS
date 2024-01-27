#include "lib/math.h"

double modf(double value, double* intPart) {
    if (intPart != NULL) {
        *intPart = (value >= 0) ? floor(value) : ceil(value);
    }
    return value - (*intPart);
}

double fmod(double dividend, double divisor) {
    if (divisor == 0) {
        return 0.0;
    }

    double quotient = dividend / divisor;
    int intPart = (int) quotient; // Integer part
    double fractionalPart = quotient - intPart; // Fractional part

    double result = fractionalPart * divisor;
    return result;
}
