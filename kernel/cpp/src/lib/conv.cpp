#include "conv.hpp"

std::string* std::itoa(ssize_t num, uint8_t base) {
    std::string* temp = new std::string();
    char alphabet[16] = {
        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
        'A', 'B', 'C', 'D', 'E', 'F'
    };

    bool negative = false;

    if(num < 0) {
        negative = true;
        num = -num;
    }

    while(num / base > 0) {
        *temp += alphabet[num % base];
        num /= base;
    }

    *temp += alphabet[num % base];

    if(negative)
        *temp += '-';

    temp->reverse();

    return temp;
}

std::string* std::itoa(ssize_t num) {
    return std::itoa(num, 10);
}