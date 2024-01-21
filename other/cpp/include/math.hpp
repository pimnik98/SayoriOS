#pragma once
#include "stdint.hpp"

#define MIN(a, b) ((a) > (b) ? (b) : (a))

namespace math {
    size_t ipow(size_t val, size_t exp);
}