#include <kernel.h>
#include "desktop/widget.h"
#include "desktop/widget_image.h"
#include "fmt/tga.h"
#include "io/serial_port.h"
#include "io/rgb_image.h"
#include "sys/pixfmt.h"

void destroy_widget_image(Widget_t* widget);

void widget_image_renderer(struct Widget* this, struct Window* window) {
    Widget_Image_t* this_data = (Widget_Image_t*)(this->custom_widget_data);

    if(this_data->image_data == NULL)
        return;

    size_t real_width = this_data->meta.w;
    size_t real_height = this_data->meta.h;

    draw_rgb_image(this_data->image_data, real_width, real_height, 32, this->x, this->y);
//    drawRect(this->x, this->y, real_width, real_height, 0xff0000);
}

Widget_t* new_widget_image(const char *path) {
    Widget_t* wgt = new_bare_widget(
        &widget_image_renderer,
        &destroy_widget_image,
        0, 0,
        0, 0
    );

    tga_header_t hdr;

    bool ok = tga_extract_info(path, &hdr);

    wgt->custom_widget_data = kcalloc(sizeof(Widget_Image_t), 1);
    Widget_Image_t* wgt_data = (Widget_Image_t*)wgt->custom_widget_data;

    if(!ok) {
        wgt_data->loaded = false;

        return wgt;
    }

    wgt_data->path = path;
    wgt_data->display_mode = NORMAL;
    wgt_data->meta = hdr;

    wgt->width = hdr.w;
    wgt->height = hdr.h;

    void* buffer_image = kcalloc(hdr.w * hdr.h, 4);
    wgt_data->image_data = buffer_image;

    tga_extract_pixels(path, buffer_image);

    pixfmt_conv(buffer_image, 32, hdr.w, hdr.h, SCREEN_BGR, SCREEN_RGB);

//    hexview_advanced(buffer_image, 512, 24, true, new_qemu_printf);

    qemu_ok("Loaded successfully!");

    return wgt;
}

void destroy_widget_image(Widget_t* widget) {
	Widget_Image_t* wgt_data = (Widget_Image_t*)(widget->custom_widget_data);

    kfree(wgt_data->image_data);
    kfree(widget->custom_widget_data);

    qemu_ok("Destroyed image widget");
}
