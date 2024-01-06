#pragma once

#include "desktop/widget.h"
#include "desktop/widget_label.h"
#include "gui/basics.h"
#include "io/serial_port.h"

typedef struct Widget_Label {
    const char* label;
	size_t length;
    uint32_t color;
} Widget_Label_t;

void widget_label_renderer(struct Widget* this, __attribute__((unused)) struct Window* container);
Widget_t* new_widget_label(const char *label, ssize_t x, ssize_t y, uint32_t color);
void destroy_widget_label(Widget_t* widget);