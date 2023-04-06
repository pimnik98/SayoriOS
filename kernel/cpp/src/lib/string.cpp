#include "stdio.hpp"
#include "string.hpp"
#include "memory.hpp"

using namespace std;

extern "C" size_t strlen(const char*);

string::string() {
    c_str = (char*)memory::alloc(1);
    length = 0;
    c_str[0] = 0;
}

string::string(const char* str) {
    length = strlen(str);

    c_str = (char*)memory::alloc(length + 1);
    memory::copy(c_str, str, length);

    c_str[length] = 0;
}

void string::add(const char chr) {
    size_t newlength = length + 1;
    
    // Length of new string with zero-terminating character
    char* new_ptr = (char*)memory::alloc(newlength + 1);

    // Copy old string
    memory::copy(new_ptr, c_str, length);

    // Set our character
    new_ptr[length] = chr;

    // Set null-terminating character
    new_ptr[newlength] = 0;

    // Free old pointer
    memory::free(c_str);

    // Set new pointer
    c_str = new_ptr;

    // Set length
    length = newlength;
}

void string::reverse() {
    size_t i = 0;
    size_t lend2 = length / 2;
    
    for (; i < lend2; i++)  {  
        swap(c_str[i], c_str[length - i - 1]);
    }  
}

string::~string() {
    memory::free(c_str);
}