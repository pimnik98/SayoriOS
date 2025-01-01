#include "desktop/widget.h"
#include "desktop/widget_label.h"
#include "lib/string.h"
#include "drv/psf.h"
#include "io/ports.h"
#include "mem/vmm.h"

void widget_label_renderer(struct Widget* this, __attribute__((unused)) struct Window* container) {
    Widget_Label_t* this_object = (Widget_Label_t*)(this->custom_widget_data);
    
    // uint32_t color = getColorFont();
    // setColorFont(this_object->color);

    draw_vga_str(this_object->label,
//				 this_object->length,
				 strlen(this_object->label),
				 this->x, this->y, this_object->color);
    // setColorFont(color);
}

Widget_t* new_widget_label(const char *label, size_t x, size_t y, uint32_t color) {
    Widget_t* wgt = new_bare_widget(
        &widget_label_renderer,
        &destroy_widget_label,
        x, y,
        1, 1
    );
    
    qemu_log("DESTROY WIDGET Label AT: %x", wgt->destroyer);

    wgt->custom_widget_data = kcalloc(sizeof(Widget_Label_t), 1);
    qemu_log("Allocated %d bytes for custom data for Widget Label (%x)", sizeof(Widget_Label_t), wgt->custom_widget_data);
    
    Widget_Label_t* wgt_data = (Widget_Label_t*)wgt->custom_widget_data;
    wgt_data->label = label;
    wgt_data->color = color;
	wgt_data->length = strlen(label);

    qemu_log("Okay?");

    return wgt;
}

void destroy_widget_label(Widget_t* widget) {
    qemu_log("Widget Label destroy its data at: %x", widget->custom_widget_data);
    kfree(widget->custom_widget_data);
}
