#include "common.h"

double trapezoidal_rule(double (*f)(double), double a, double b, size_t steps) {
    double h = (b - a) / steps;
    double sum = 0.5 * (f(a) + f(b));

    for (size_t i = 1; i < steps; i++) {
        double x = a + (double)i * h;
        sum += f(x);
    }

    return h * sum;
}