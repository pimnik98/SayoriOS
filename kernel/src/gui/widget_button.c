#include <kernel.h>
#include "gui/widget.h"
#include "gui/widget_button.h"
#include "gui/basics.h"
#include "io/port_io.h"

void destroy_widget_button(Widget_t* widget);

void widget_button_renderer(struct Widget* this, struct Window* container) {
    Widget_Button_t* this_object = (Widget_Button_t*)(this->custom_widget_data);
    
    /*
    qemu_log("DATA: W: %d H: %d X: %d Y: %d COLOR: %x LABEL_COLOR: %x LABEL_ADDRESS: %x",
                this->width, this->height,
                this->x, this->y,
                this_object->color, this_object->label_color,
                this_object->label);*/
    // qemu_log("WIDGET AT: %x; IT's DATA: %x", this, this->custom_widget_data);

    draw_rectangle(this->x, this->y, this->width, this->height, 0);
    draw_rectangle(this->x-1, this->y-1, this->width+1, this->height+1, 0);
    draw_filled_rectangle(
        this->x, this->y,
        this->width, this->height, 
        this_object->color
    );

    draw_vga_str(this_object->label, strlen(this_object->label),
        this->x + 5,
        this->y + (this->height - 16)/2,
        this_object->label_color
    );
    // setColorFont(color);
}

Widget_t* new_widget_button(char* label, uint32_t color, uint32_t label_color) {
    Widget_t* wgt = new_bare_widget(
        &widget_button_renderer,
        &destroy_widget_button,
        0, 0,
        8*(strlen(label)), 10
    );

    wgt->custom_widget_data = kcalloc(sizeof(Widget_Button_t), 1);
    
    Widget_Button_t* wgt_data = (Widget_Button_t*)wgt->custom_widget_data;
    wgt_data->label = label;
    wgt_data->label_color = label_color;
    wgt_data->color = color;

    qemu_log("Created Widget Button at: %x", wgt);
    qemu_log("Created Widget Button DATA at: %x", wgt->custom_widget_data);

    return wgt;
}

void destroy_widget_button(Widget_t* widget) {
    qemu_log("Widget button destroy its data at: %x", widget->custom_widget_data);
    kfree(widget->custom_widget_data);
}

