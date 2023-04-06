#pragma once

#include <common.h>

typedef struct Widget_Button {
    char* label;
    uint32_t label_color;
    uint32_t color;
} Widget_Button_t;

Widget_t* new_widget_button(char* label, uint32_t color, uint32_t label_color);