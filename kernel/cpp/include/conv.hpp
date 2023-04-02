#pragma once

#include "string.hpp"

namespace std {
    std::string* itoa(ssize_t num, uint8_t base);
    std::string* itoa(int number);
}