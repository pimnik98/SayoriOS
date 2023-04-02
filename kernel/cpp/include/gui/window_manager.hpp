#pragma once

#include "gui/window.hpp"
#include "vector.hpp"
#include "display.hpp"

namespace GUI {
    class WindowManager {
        public:

            WindowManager();
            ~WindowManager();

            void add_window(Window& window);
            void remove(size_t idx);

            void render_once();

        private:

            void render_window(Window window);

            std::display display;
            std::vector<Window> windows;
    };
}