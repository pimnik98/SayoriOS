#include "lib/math.h"

double exp(double x) {
    double result = 1.0;
    double term = 1.0;
    
    for (int n = 1; n < 200; n++) {
        term *= x / n;
        result += term;
    }
    
    return result;
}
