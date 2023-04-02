#pragma once

#include "memory.hpp"

extern "C" {
    size_t strlen(const char*);
    int strcmp(const char *s1, const char *s2);
}

namespace std {
    template <typename T>
    void swap(T& a, T& b) {
        char tmp = a;
        a = b;
        b = tmp;
    }

    class string {
        public:
            string();
            string(const char*);

            // Move
            string operator=(string a) { return a; }
            // Create new from charptr
            string operator=(const char* a) {
                return string(a);
            }

            // Simple ADD operator overload.

            string& operator+(const char b) {
                this->add(b);

                return *this;
            }

            string& operator+(const char* b) {
                size_t newlength = length + strlen(b);

                char* tmp = (char*)memory::alloc(newlength + 1);
                
                memory::copy(tmp, c_str, length);
                memory::copy(tmp + length, b, strlen(b));

                tmp[newlength] = 0;

                memory::free(c_str);
                c_str = tmp;
                length = newlength;

                return *this;
            }

            string& operator+(const string& b) {
                return this->operator+(b.c_str);
            }

            // PLUS-EQUAL operator overload.

            template <typename T>
            string& operator+=(T b) {
                return this->operator+(b);
            }

            void add(const char);
            size_t get_length() const { return length; }
            void reverse();

            ~string();
        
            char* c_str;

        private:
        size_t length = 0;
    };
}