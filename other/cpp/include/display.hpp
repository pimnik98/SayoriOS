#pragma once

#include "stdint.hpp"

namespace std {
    struct pixel_rgb {
        pixel_rgb(uint8_t r, uint8_t g, uint8_t b)
        : r(r), g(g), b(b) {}

        uint8_t r;
        uint8_t g;
        uint8_t b;
    };

    struct Vec2d {
        ssize_t x;
        ssize_t y;
    };

    class display {
        public:
        
        display();
        ~display();

        void init();

        // Draw pixel from the `pixel_rgb` structure.
        void draw_pixel(Vec2d position, pixel_rgb& pixel);

        // Draw pixel from R, G and B.
        void draw_pixel(Vec2d position, uint8_t r, uint8_t g, uint8_t b);

        // Same functions, use x and y instead of `Vec2d` struct.
        void draw_pixel(size_t x, size_t y, pixel_rgb& pixel);
        void draw_pixel(size_t x, size_t y, uint8_t r, uint8_t g, uint8_t b);

        // Update display contents.
        void flip();
        
        // Get pixel at the position.
        size_t get_pixel(Vec2d position);
        size_t get_pixel(size_t x, size_t y);

        void draw_rectangle(ssize_t x, ssize_t y, size_t width, size_t height, uint8_t r, uint8_t g, uint8_t b);
        void draw_filled_rect(ssize_t x, ssize_t y, size_t width, size_t height, uint8_t r, uint8_t g, uint8_t b);

        const size_t width();
        const size_t height();

        private:
            size_t _width;
            size_t _height;

            uint8_t* _fbaddr;
            uint8_t* _rdispaddr;
            size_t _fbpitch;
            size_t _fbbpp;
    };
}