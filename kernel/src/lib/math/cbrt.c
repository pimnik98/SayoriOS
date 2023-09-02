#include "lib/math.h"

double cbrt(double x) {
    double guess = x; // Initial guess
    double epsilon = 1e-7; // Tolerance level
    
    while ((guess * guess * guess - x) > epsilon) {
        guess = (2 * guess + x / (guess * guess)) / 3;
    }
    
    return guess;
}
