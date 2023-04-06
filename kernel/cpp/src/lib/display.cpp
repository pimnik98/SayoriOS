#include <stdint.hpp>
#include "display.hpp"

extern "C" {
    size_t getWidthScreen();
    size_t getHeightScreen();
    size_t getDisplayPitch();
    size_t getDisplaySize();
    size_t getFrameBufferAddr();
    size_t getDisplayBpp();

    void punch();
}

using namespace std;

display::display() {
    _fbaddr = (uint8_t*)getFrameBufferAddr();
    
    _width = getWidthScreen();
    _height = getHeightScreen();

    _fbpitch = getDisplayPitch();
    _fbbpp = getDisplayBpp();
}

display::~display() {}

const size_t display::width() {
    return getWidthScreen();
}

const size_t display::height() {
    return getHeightScreen();
}

void display::flip() {
    punch();
}

void display::draw_pixel(ssize_t x, ssize_t y, uint8_t r, uint8_t g, uint8_t b) {
    if (x < 0 || y < 0 ||
        x >= _width ||
        y >= _height) {
            return;
    }

    uint8_t* pixels = (uint8_t*)(_fbaddr + (x * (_fbbpp / 8)) + y * _fbpitch);

    pixels[0] = b;
    pixels[1] = g;
    pixels[2] = r;
}

void display::draw_pixel(ssize_t x, ssize_t y, pixel_rgb& pixel) {
    display::draw_pixel(x, y, pixel.r, pixel.g, pixel.b);
}

void display::draw_pixel(Vec2d position, uint8_t r, uint8_t g, uint8_t b) {
    display::draw_pixel(position.x, position.y, r, g, b);
}

void display::draw_pixel(Vec2d position, pixel_rgb& pixel) {
    display::draw_pixel(position.x, position.y, pixel);
}

uint32_t display::get_pixel(size_t x, size_t y) {
    if (x < 0 || y < 0 ||
        x >= _width ||
        y >= _height) {
        return 0x000000;
    }

    size_t where = x * (_fbbpp / 8) + y * _fbpitch;

    return ((_fbaddr[where+2] & 0xff) << 16) + ((_fbaddr[where+1] & 0xff) << 8) + (_fbaddr[where] & 0xff);
}

void display::draw_filled_rect(ssize_t x, ssize_t y, size_t width, size_t height, uint8_t r, uint8_t g, uint8_t b) {
    for(size_t ay = 0; ay < height; ay++){
        for(size_t ax = 0; ax < width; ax++){
            display::draw_pixel(ax + x, ay + y, r, g, b);
        }
    }
}
