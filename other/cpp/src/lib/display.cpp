#include <stdint.hpp>
#include "display.hpp"
#include "memory.hpp"
#include "log.hpp"

extern "C" {
    size_t getWidthScreen();
    size_t getHeightScreen();
    size_t getDisplayPitch();
    size_t getDisplaySize();
    size_t getFrameBufferAddr();
    size_t getDisplayAddr();
    size_t getDisplayBpp();
}

using namespace std;

display::display() {
    init();
}

display::~display() {}

void display::init() {
    _fbaddr = (uint8_t*)getFrameBufferAddr();
    _rdispaddr = (uint8_t*)getDisplayAddr();
    
    _width = getWidthScreen();
    _height = getHeightScreen();

    _fbpitch = getDisplayPitch();
    _fbbpp = getDisplayBpp();
}

const size_t display::width() {
    return getWidthScreen();
}

const size_t display::height() {
    return getHeightScreen();
}

void display::flip() {
    memcpy(this->_rdispaddr, this->_fbaddr, this->_height * this->_fbpitch);
}

void display::draw_pixel(size_t x, size_t y, uint8_t r, uint8_t g, uint8_t b) {
    if (x >= _width ||
        y >= _height) {
            // std::log << "Draw overflow" << endl;
            // std::log << _width << endl;
            // std::log << _height << endl;
            return;
    }

    uint8_t* pixels = _fbaddr + (x * (_fbbpp >> 3)) + y * _fbpitch;

    pixels[0] = b;
    pixels[1] = g;
    pixels[2] = r;
}

void display::draw_pixel(size_t x, size_t y, pixel_rgb& pixel) {
    display::draw_pixel(x, y, pixel.r, pixel.g, pixel.b);
}

void display::draw_pixel(Vec2d position, uint8_t r, uint8_t g, uint8_t b) {
    display::draw_pixel(position.x, position.y, r, g, b);
}

void display::draw_pixel(Vec2d position, pixel_rgb& pixel) {
    display::draw_pixel(position.x, position.y, pixel);
}

uint32_t display::get_pixel(size_t x, size_t y) {
    if (x >= _width ||
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

void display::draw_rectangle(ssize_t x, ssize_t y, size_t width, size_t height, uint8_t r, uint8_t g, uint8_t b) {    
    for(int i = x, to = x + width; i < to; i++) {
        display::draw_pixel(i, y, r, g, b);
        display::draw_pixel(i, y + height, r, g, b);
    }
	
    for(int j = y, to2 = y + height; j < to2; j++) {
        display::draw_pixel(x, j, r, g, b);
        display::draw_pixel(x + width, j, r, g, b);
    }
}