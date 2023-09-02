#include "lib/math.h"

double pow(double base, double exponent) {
    if (base == 0) {
        return 0; // 0^exponent is 0
    } else if (base == 1 || exponent == 0) {
        return 1; // Anything raised to the power of 0 or 1^exponent is 1
    } else {
        return exp(exponent * log(base));
    }
}