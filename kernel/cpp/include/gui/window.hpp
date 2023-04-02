#pragma once

#include "string.hpp"
#include "stdint.hpp"

namespace GUI {
    class Window {
        public:
        
            Window();
            ~Window();

            void set_name(char* name);
            void set_name(std::string name);

            void move(size_t x, size_t y);
            void resize(size_t width, size_t height);

            void pixel(size_t x, size_t y, uint32_t color);

            uint8_t* get_buffer() const;

            size_t get_width() const;
            size_t get_height() const;
            size_t get_x() const;
            size_t get_y() const;

        private:

            size_t measure_buffer_size();
            
            uint8_t* buffer;

            size_t x, y, width, height;
            char* name;
    };
}