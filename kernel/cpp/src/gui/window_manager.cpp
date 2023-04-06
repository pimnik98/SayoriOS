#include "gui/window_manager.hpp"

using namespace GUI;

WindowManager::WindowManager() {
    
}

WindowManager::~WindowManager() {
    
}

void WindowManager::add_window(Window& window) {
    windows.push_back(window);
}

void WindowManager::remove(size_t idx) {
    windows.remove(idx);
}

void WindowManager::render_once() {
    for(size_t y = 0; y < display.height(); y++) {
        for(size_t x = 0; x < display.width(); x++) {
            display.draw_pixel(x, y, 90, 90, 90);
        }
    }
    
    for(size_t i = 0, len = windows.get_length(); i < len; i++) {
        render_window(windows[i]);
    }

    display.flip();
}

void WindowManager::render_window(Window window) {
    auto buffer = window.get_buffer();
    auto xpos = window.get_x();
    auto ypos = window.get_y();

    display.draw_filled_rect(xpos, ypos, window.get_width(), 25, 0xaa, 0xaa, 0xaa);

    ypos += 25;

    for(size_t y = 0, h = window.get_height(); y < h; y++) {
        for(size_t x = 0, w = window.get_width(); x < w; x++) {
            auto pos = x * 3 + (y * 3 * w);
            
            display.draw_pixel(
                xpos + x,
                ypos + y,
                buffer[pos + 2],
                buffer[pos + 1],
                buffer[pos + 0]
            );
        }
    }
}