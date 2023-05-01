#include "gui/render.h"
#include "gui/widget_button.h"
#include "gui/widget_image.h"
#include "gui/widget_label.h"

void eki_start() {
    Window_t* eki_mainwindow = window_new("Eki");
    eki_mainwindow->x = 100;
    eki_mainwindow->y = 100;
    eki_mainwindow->width = 200;
    eki_mainwindow->height = 200;
    eki_mainwindow->canvas_bgcolor = 0x333333;

    Widget_t* hello_button = new_widget_button("Hi", 0x00ff00, 0x000000);
    hello_button->x = 50;
    hello_button->y = 50;
    hello_button->width  += 30;
    hello_button->height += 15;

    window_add_widget(eki_mainwindow, hello_button);

    eki_mainwindow->state = DISPLAYING;
}