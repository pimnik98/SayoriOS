#include "gui/widget.h"
#include "io/port_io.h"

Widget_t* new_bare_widget(renderer_func_t renderer, destroyer_func_t destroyer, size_t x, size_t y, size_t width, size_t height) {
    Widget_t* wgt = kcalloc(sizeof(Widget_t), 1);
    wgt->x = x;
    wgt->y = y;
    wgt->width = width;
    wgt->height = height;
    wgt->renderer = renderer;
    wgt->destroyer = destroyer;

    wgt->on_click = 0;

    return wgt;
}

void destroy_widget(Widget_t* widget) {
    qemu_log("Destroying widget at %x: W: %d; H: %d", widget, widget->width, widget->height);
    widget->destroyer(widget);
    kfree(widget);
    widget = 0;
}

void widget_notify(struct Window* window, struct Widget* widget, WidgetNotifyCode_t code, void* data) {
    qemu_log("Reached widget_notify()");
    qemu_log("Got WIDGET notifcation: (WINDOW@%v)(id: %d) (WIDGET@%v) (CODE: %s)",
        window,
        window->id,
        widget,
        (code == WIDGET_CLICK ?
            "WIDGET_CLICK":
            "UNKNOWN"
        )
    );

    if(code == WIDGET_CLICK) {
        qemu_log("Code check: onclick at %x", widget->on_click);
        if(widget->on_click) {
            qemu_log("On Click: %x", widget->on_click);
            widget->on_click(widget, (Coordinates_t*)data);
            qemu_log("Returning from on_click");
        }else{
            qemu_log("Function widget->on_click not defined!!!");
        }
    }
}