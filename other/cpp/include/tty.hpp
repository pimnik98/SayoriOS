#pragma once

#include "math.hpp"
#include "string.hpp"
#include "display.hpp"

namespace internals {
    class EndLine {
        public:
        EndLine() {}
        ~EndLine() {}
    };

    class TTY {
        public:

        std::display display;

        void buffered_write(const char* bytes);
        void buffered_write(char character1, char character2 = 0);
        void buffered_write(const int num);
        void buffered_write(const unsigned int num);
        void buffered_write(double num, size_t after_dot = 5);
        void buffered_write(const void* ptr);

        void write(const char* bytes);
        void write(char character1, char character2);
        void write(const int num);
        void write(const unsigned int num);
        void write(double num);
        void write(const std::string& str);
        void write(const void* ptr);
        void write(const EndLine& endl);

        template <typename T>
        TTY& operator << (T t) {
            write(t);
            return *this;
        }
    };

    inline void TTY::write(const char* bytes) {
        this->buffered_write(bytes);
        this->display.flip();
    }

    inline void TTY::write(char character1, char character2) {
        this->buffered_write(character1, character2);
        this->display.flip();
    }

    inline void TTY::write(const int num) {
        this->buffered_write(num);
        this->display.flip();
    }

    inline void TTY::write(const unsigned int num) {
        this->buffered_write(num);
        this->display.flip();
    }

    inline void TTY::write(double num) {
        this->buffered_write(num);
        this->display.flip();
    }

    inline void TTY::write(const std::string& str) {
        this->buffered_write(str.c_str);
        this->display.flip();
    }
    
    inline void TTY::write(const void* ptr) {
        this->buffered_write(ptr);
        this->display.flip();
    }
    
    inline void TTY::write(const EndLine& ptr) {
        this->buffered_write("\n");
        this->display.flip();
    }
}