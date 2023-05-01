#ifndef MATH_H
#define MATH_H

#include "common.h"

#define MIN(a, b) (a > b ? b : a)
#define MAX(a, b) (a < b ? b : a)

#define PI 3.141592653589793f

size_t fac(size_t);
double pow(double, size_t);
double deg2rad(size_t);
double sin(double);
double cos(double);
double tan(double);

#endif
