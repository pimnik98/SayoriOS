#pragma once

#include "gui/widget.h"
#include "gui/widget_label.h"
#include "gui/basics.h"
#include "io/port_io.h"

typedef struct Widget_Label {
    char* label;
    uint32_t color;
} Widget_Label_t;

void widget_label_renderer(struct Widget* this, struct Window* container);
Widget_t* new_widget_label(char* label, ssize_t x, ssize_t y, uint32_t color);
void destroy_widget_label(Widget_t* widget);