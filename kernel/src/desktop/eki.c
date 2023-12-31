#include "desktop/render.h"
#include "desktop/widget_button.h"
#include "desktop/widget_image.h"
#include "desktop/widget_label.h"
#include "drv/disk/dpm.h"
#include "io/ports.h"
#include "../lib/libstring/include/string.h"

extern DPM_Disk DPM_Disks[32];

void eki_on_close(Window_t* window);

void eki_start() {
    vector_t* eki_strings = vector_new();

    Window_t* eki_mainwindow = window_new("Eki");
    eki_mainwindow->x = 100;
    eki_mainwindow->y = 100;
    eki_mainwindow->width = 200;
    eki_mainwindow->height = 200;
    eki_mainwindow->canvas_bgcolor = 0x333333;
    eki_mainwindow->on_close = eki_on_close;
    eki_mainwindow->data = eki_strings;

    int yoffset = 0;

    for (int i = 0; i < 32; i++) {
        if (DPM_Disks[i].Ready) {
            yoffset++;

            string_t* tempstr = string_new();
            string_append_char(tempstr, 'A' + i);

            vector_push_back(eki_strings, (size_t) tempstr);

            Widget_t* hello_button = new_widget_button(tempstr->data, 0x00ff00, 0x000000);
            hello_button->x = 10;
            hello_button->y = 10 + (yoffset * 20);
            hello_button->width  += 30;
            hello_button->height += 15;

            window_add_widget(eki_mainwindow, hello_button);
        }
    }

    eki_mainwindow->state = DISPLAYING;
}

void eki_on_close(Window_t* window) {
    qemu_log("EKI close!");

    vector_t* that_vec = (vector_t*)window->data;

    for(int i = 0; i < that_vec->size; i++) {
        string_destroy((string_t*)vector_get(that_vec, i).element);
    }

    vector_destroy(that_vec);
}