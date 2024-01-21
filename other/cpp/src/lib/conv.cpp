#include "conv.hpp"

std::string* std::itoa(ssize_t num, uint8_t base) {
    auto* temp = new std::string();
    char* alphabet = (char*)"0123456789ABCDEF";

    bool negative = false;

    if(num < 0) {
        negative = true;
        num = -num;
    }

    do {
        *temp += alphabet[num % base];
        num /= base;
    } while (num > 0);

    *temp += alphabet[num % base];

    if(negative)
        *temp += '-';

    temp->reverse();

    return temp;
}

std::string* std::itoa(ssize_t num) {
    return std::itoa(num, 10);
}