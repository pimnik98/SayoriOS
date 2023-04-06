#include "math.hpp"

size_t math::ipow(size_t val, size_t exp) {
    size_t value = val;
    for (size_t i = 1; i < exp; i++) {
        value *= val;
    }

    return value;
}