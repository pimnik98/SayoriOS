#include "lib/math.h"

double sqrt(double x) {
    if (x < 0) {
        return -1; // Square root is undefined for negative numbers
    }
    
    double guess = x; // Initial guess
    double epsilon = 1e-7; // Tolerance level

    while ((guess * guess - x) > epsilon) {
        guess = 0.5 * (guess + x / guess);
    }
    
    return guess;
}