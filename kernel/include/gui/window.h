#pragma once

#include <common.h>
// #include "gui/widget.h"
#include "gui/pointutils.h"

typedef enum WindowState {
    NOT_INITIALIZED,
    HIDDEN,
    DISPLAYING
} WindowState_t;

typedef enum WindowSignal {
    WINDOW_CLOSE,
    WINDOW_CLICK,
    WINDOW_MINIMIZE
} WindowSignal_t;

typedef struct Window {
    size_t id;
    char* title;

    size_t width, height;
    ssize_t x, y;

    bool with_title_bar;
    WindowState_t state;
    bool minimizable;
    bool closable;

    uint32_t canvas_bgcolor;

    size_t widget_count;
    struct Widget** widgets;
} Window_t;

Window_t* window_new(char* title);
Window_t** get_window_list();
void _window_add_to_list(Window_t* window);
size_t get_window_count();
void window_destroy(Window_t* win);
void window_add_widget(Window_t* window, struct Widget* widget);
void window_remove_widget(Window_t* window, struct Widget* widget);
void window_send_signal(Window_t* window, WindowSignal_t signal, void* data);
void destroy_all_windows();