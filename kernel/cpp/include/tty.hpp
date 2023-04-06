#pragma once

#include "math.hpp"
#include "string.hpp"

namespace internals {
    class TTY {
        public:

        void buffered_write(const char* bytes);
        void buffered_write(char character1, char character2 = 0);
        void buffered_write(const int num);
        void buffered_write(double num, size_t after_dot = 5);
        void buffer_flush();

        void write(const char* bytes);
        void write(char character1, char character2 = 0);
        void write(const int num);
        void write(double num);
        void write(const std::string& str);

        template <typename T>
        TTY& operator << (T t) {
            write(t);
            return *this;
        }
    };
}