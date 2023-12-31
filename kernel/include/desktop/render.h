#pragma once

#include "desktop/window.h"
#include "desktop/widget.h"
#include "gui/basics.h"
#include "io/tty.h"

#define WINDOW_TITLEBAR_HEIGHT 22
#define WINDOW_CANVAS_COLOR    0x202020
#define WINDOW_TITLEBAR_COLOR  0x909090
#define GUI_BACKGROUND_COLOR   0x505050

extern vector_t* windows;

void gui_render_window(Window_t* window);
// Render GUI
void gui_render();