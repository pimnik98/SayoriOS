#include "memory.hpp"
#include "gui/window.hpp"
#include "log.hpp"
#include "stdio.hpp"

using namespace GUI;

Window::Window() {
    this->x = 100;
    this->y = 100;

    this->width = 100;
    this->height = 100;

    this->name = "Untitled window";

    // buffer = new uint8_t[measure_buffer_size()];
    this->buffer = (uint8_t*)memory::alloc(measure_buffer_size());
    memory::set(this->buffer, (uint8_t)0, measure_buffer_size());

    std::log << "Initialized window: WIDTH: " << this->width << " HEIGHT: " << this->height << " NAME: " << this->name << std::endl;
    std::log << "Buffer size is: " << measure_buffer_size() << std::endl;
}

Window::~Window() {
    delete[] buffer;
}

size_t Window::measure_buffer_size() {
    return (this->width * this->height * 3);
}

void Window::resize(size_t width, size_t height) {
    uint8_t* new_buffer = new uint8_t[width * height * 3];

    memory::copy(new_buffer, buffer, measure_buffer_size());

    delete[] buffer;

    buffer = new_buffer;

    this->width = width;
    this->height = height;
}

void Window::move(size_t x, size_t y) {
    this->x = x;
    this->y = y;
}

void Window::set_name(std::string str) {
    this->name = str.c_str;
}

void Window::set_name(char* str) {
    this->name = str;
}


void Window::pixel(size_t x, size_t y, uint32_t color) {
    // std::qemu_log("Drawing on ");
    // std::qemu_log(x);
    // std::qemu_log(" ");
    // std::qemu_log(y);
    // std::qemu_log("\n");

    // std::log << "Drawing on: " << x << " " << y << "\n";

    buffer[x * 3 + (y * 3 * width)] = color & 0xff;
    buffer[x * 3 + (y * 3 * width) + 1] = (color >> 8) & 0xff;
    buffer[x * 3 + (y * 3 * width) + 2] = (color >> 16) & 0xff;
}

uint8_t* Window::get_buffer() const {
    return buffer;
}

size_t Window::get_width() const {
    return width;
}

size_t Window::get_height() const {
    return height;
}

size_t Window::get_x() const {
    return x;
}

size_t Window::get_y() const {
    return y;
}