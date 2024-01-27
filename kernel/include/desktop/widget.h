#pragma once

#include <common.h>
#include "desktop/window.h"
#include "gui/pointutils.h"

struct Widget;

typedef void (*renderer_func_t)(struct Widget*, struct Window*);
typedef void (*destroyer_func_t)(struct Widget*);

typedef struct Widget {
    renderer_func_t renderer;
    destroyer_func_t destroyer;

    size_t x, y;
    size_t width, height;

    void (*on_click)(struct Widget* this, Coordinates_t* coords);

    void* custom_widget_data;
} Widget_t;

typedef enum WidgetNotifyCode {
    WIDGET_CLICK,
    WIDGET_MOVE
} WidgetNotifyCode_t;

Widget_t* new_bare_widget(renderer_func_t renderer, destroyer_func_t destroyer, size_t x, size_t y, size_t width, size_t height);
void window_add_widget(Window_t* window, struct Widget* widget);
void window_remove_widget(Window_t* window, struct Widget* widget);
void destroy_widget(Widget_t* widget);
void widget_notify(struct Window* window, struct Widget* widget, WidgetNotifyCode_t code, void* data);