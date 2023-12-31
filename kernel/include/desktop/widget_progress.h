#pragma once

#include "desktop/widget.h"
#include "gui/basics.h"
#include "io/serial_port.h"

typedef struct Widget_Progress {
    size_t current;
} Widget_Progress_t;

void widget_progress_renderer(struct Widget* this, __attribute__((unused)) struct Window* container);
Widget_t* new_widget_progress();
void destroy_widget_progress(Widget_t* widget);