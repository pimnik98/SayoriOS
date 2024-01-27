#include "desktop/widget.h"
#include "desktop/widget_progress.h"
#include "gui/basics.h"
#include "mem/vmm.h"

void widget_progress_renderer(struct Widget* this, __attribute__((unused)) struct Window* container) {
//    Widget_Progress_t* this_object = (Widget_Progress_t*)(this->custom_widget_data);

    size_t cost = this->width / 100;

    if(((Widget_Progress_t*)this->custom_widget_data)->current > 100) {
        ((Widget_Progress_t*)this->custom_widget_data)->current = 100;
    }

    draw_rectangle(this->x, this->y, this->width, this->height, 0x000000);
    draw_filled_rectangle(
            this->x + 2,
            this->y + 2,
            (cost * ((Widget_Progress_t*)this->custom_widget_data)->current),
            this->height - 4,
            0xffffff);
}

Widget_t* new_widget_progress() {
    Widget_t* wgt = new_bare_widget(
        &widget_progress_renderer,
        &destroy_widget_progress,
        0, 0,
        1, 1
    );

    wgt->custom_widget_data = kcalloc(sizeof(Widget_Progress_t), 1);

    return wgt;
}

void destroy_widget_progress(Widget_t* widget) {
    kfree(widget->custom_widget_data);
}
