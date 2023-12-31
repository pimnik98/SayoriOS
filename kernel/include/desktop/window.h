#pragma once

#include <common.h>
#include "gui/pointutils.h"
#include "../../src/lib/libvector/include/vector.h"

#define WINDOW(idx) ((Window_t*)(windows->data[idx]))

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

struct Window;

typedef void (*on_close_function_t)(struct Window* window);

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

    vector_t* widgets;

    on_close_function_t on_close;
    void* data;  // Data, used by application to unify window's data
} Window_t;

Window_t* window_new(char* title);
vector_t * get_window_list();
void _window_add_to_list(Window_t* window);
size_t get_window_count();
void window_destroy(Window_t* win);
void window_send_signal(Window_t* window, WindowSignal_t signal, void* data);
void destroy_all_windows();