#include "gui/widget.h"
#include "io/imaging.h"
#include "gui/widget_image.h"
#include "io/imaging.h"
#include "io/port_io.h"

void destroy_widget_image(Widget_t* widget);

void widget_image_renderer(struct Widget* this, struct Window* window) {
    Widget_Image_t* this_data = (Widget_Image_t*)(this->custom_widget_data);
    
    duke_rawdraw(this_data->image_data, this_data->meta, this->x, this->y);
}

Widget_t* new_widget_image(char* path) {
	struct DukeImageMeta* metadata = kcalloc(sizeof(struct DukeImageMeta), 1);
    char code = duke_get_image_metadata(path, metadata);

    // qemu_log("Load image metadata code: %x", code);
    
    // qemu_log("Image: %d x %d", metadata.width, metadata.height);
    // qemu_log("Allocating %d bytes for Widget Image", metadata.data_length);
    void* buffer_image = kcalloc(metadata->data_length, 1);

    qemu_log("Getting data");
    duke_get_image_data(path, *metadata, buffer_image);
    qemu_log("Okay");

    Widget_t* wgt = new_bare_widget(
        &widget_image_renderer,
        &destroy_widget_image,
        0, 0,
        metadata->width, metadata->height
    );
    
    wgt->custom_widget_data = kcalloc(sizeof(Widget_Image_t), 1);
    
    Widget_Image_t* wgt_data = (Widget_Image_t*)wgt->custom_widget_data;
    wgt_data->path = path;
    wgt_data->display_mode = NORMAL;
    wgt_data->meta = metadata;
    wgt_data->image_data = buffer_image;

    return wgt;
}

void destroy_widget_image(Widget_t* widget) {
	Widget_Image_t* wgt_data = (Widget_Image_t*)(widget->custom_widget_data);

    // qemu_log("Freeing image data: %x", wgt_data->image_data);
    kfree(wgt_data->image_data);
    kfree(wgt_data->meta);
    // qemu_log("Freeing widget data: %x", widget->custom_widget_data);
    kfree(widget->custom_widget_data);
    // qemu_log("Freeing widget itself: %x", widget);
    // destroy_widget(widget);
}
