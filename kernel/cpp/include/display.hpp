#pragma once

#include "stdint.hpp"

namespace std {
    typedef struct {
        uint8_t r;
        uint8_t g;
        uint8_t b;
    } pixel_rgb;

    typedef struct {
        ssize_t x;
        ssize_t y;
    } Vec2d;

    typedef struct {
        ssize_t x;
        ssize_t y;
        ssize_t z;
    } Vec3d;

    class display {
        public:
        
        display();
        ~display();

        // Draw pixel from the `pixel_rgb` structure.
        void draw_pixel(Vec2d position, pixel_rgb& pixel);

        // Draw pixel from R, G and B.
        void draw_pixel(Vec2d position, uint8_t r, uint8_t g, uint8_t b);

        // Same functions, use x and y instead of `Vec2d` struct.
        void draw_pixel(ssize_t x, ssize_t y, pixel_rgb& pixel);
        void draw_pixel(ssize_t x, ssize_t y, uint8_t r, uint8_t g, uint8_t b);

        // Update display contents.
        void flip();
        
        // Get pixel at the position.
        size_t get_pixel(Vec2d position);
        size_t get_pixel(size_t x, size_t y);

        void draw_filled_rect(ssize_t x, ssize_t y, size_t width, size_t height, uint8_t r, uint8_t g, uint8_t b);

        const size_t width();
        const size_t height();

        private:
            size_t _width;
            size_t _height;

            uint8_t* _fbaddr;
            size_t _fbpitch;
            size_t _fbbpp;
    };
}