#include "lib/math.h"

double log(double x) {
    if (x <= 0) {
        return -1;
    }

    double result = 0.0;
    double term = (x - 1) / (x + 1);
    double term_squared = term * term;
    double numerator = term;
    
    for (int n = 1; n < 200; n += 2) {
        result += numerator / n;
        numerator *= term_squared;
    }
    
    return 2 * result;
}