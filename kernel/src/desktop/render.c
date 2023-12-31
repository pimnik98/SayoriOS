#include "gui/basics.h"
#include "gui/pointutils.h"
#include "desktop/window.h"
#include "desktop/render.h"
#include "drv/psf.h"
#include "io/ports.h"
#include <mem/vmm.h>
#include "drv/input/mouse.h"

// getConfigFonts(2) - is height of current font

Window_t* active_window = 0;  // Window that should be placed over all windows (selected window) 

bool dragging_mode = false;
bool click = false;
Window_t* drag_window = 0;

uint32_t dragx = 0;
uint32_t dragy = 0;

void* cursor_data = 0;
size_t cursor_width = 0, cursor_height = 0;
bool cursor_ok = false;

Window_t* focused = 0;

char* cursor_default_path = "/var/cursor_normal.duke";

void gui_restore() {
    if(cursor_ok) {
        qemu_log("DUKE2CURSOR: Freeing cursor...");
        kfree(cursor_data);
        cursor_ok = false;
    }

    destroy_all_windows();
    clean_screen();

    // setColorFont(0xffffff);
}

void gui_render_widgets(Window_t* window) {
    for (size_t i = 0; i < window->widgets->size; i++) {
        Widget_t* wgt = (Widget_t *) window->widgets->data[i];

        wgt->x += window->x;
        wgt->y += window->y;

        wgt->renderer(wgt, window);

        wgt->x -= window->x;
        wgt->y -= window->y;
    }
}

void gui_render_window(Window_t* window) {
    if(!window) return;
    if(window->state != DISPLAYING) return;

    // Window canvas
    draw_filled_rectangle(
        window->x,
        window->y,
        window->width, 
        window->height, 
        window->canvas_bgcolor
    );

    draw_rectangle(window->x-1, window->y-1, window->width+1, window->height+1, 0x000000);

    if(window->with_title_bar) {
        // Window Titlebar
        draw_filled_rectangle(window->x, window->y - WINDOW_TITLEBAR_HEIGHT, window->width, WINDOW_TITLEBAR_HEIGHT, WINDOW_TITLEBAR_COLOR);
        // Window Title Text
        draw_vga_str(window->title, strlen(window->title), window->x + 5, window->y - ((WINDOW_TITLEBAR_HEIGHT + 16) / 2), 0);
    }

    if(window->closable) {
        draw_filled_rectangle(window->x + window->width - WINDOW_TITLEBAR_HEIGHT - 4, window->y - WINDOW_TITLEBAR_HEIGHT + 2, WINDOW_TITLEBAR_HEIGHT + 2, WINDOW_TITLEBAR_HEIGHT - 4, 0xdd0000);

        draw_vga_str("X", 1,
            window->x + window->width - WINDOW_TITLEBAR_HEIGHT + 5,
            window->y - WINDOW_TITLEBAR_HEIGHT + 4,
            0
        );
    }

    gui_render_widgets(window);
}

Window_t* is_point_on_any_window_titlebar(size_t x, size_t y) {
    for (size_t i = 0; i < get_window_count(); i++) {
        struct Window* window = WINDOW(i);

		if(window->with_title_bar && point_in_rect(
            x,
            y,
            window->x,
            window->y - WINDOW_TITLEBAR_HEIGHT,
            window->width,
            WINDOW_TITLEBAR_HEIGHT
            )) {
                return WINDOW(i);
            }
    }
    return 0;
}

Window_t* is_point_on_any_window(ssize_t x, ssize_t y) {
    for (size_t i = get_window_count()-1; i >= 0 ; i--) {
        // qemu_log("Checking #%d (%x)", current_windows[i]->id, current_windows[i]);
        if(point_in_rect(
            x,
            y,
            WINDOW(i)->x,
            WINDOW(i)->y,
            WINDOW(i)->width,
            WINDOW(i)->height
            )) {
                // qemu_log("It's: %d", current_windows[i]->id);
                return WINDOW(i);
            }
    }
    return 0;
}

size_t get_window_index(Window_t* win) {
    for(size_t i = 0, wnds = get_window_count(); i < wnds; i++){
        if(WINDOW(i) == win) {
            return i;
        }
    }

    return 0;
}

void gui_handle_mouse() {
    uint32_t mouse_color = 0xff0000;

    size_t mouse_x = mouse_get_x();
    size_t mouse_y = mouse_get_y();

    bool left_mouse = mouse_get_b1();
    bool right_mouse = mouse_get_b2();
    
    if(right_mouse){
        mouse_color |= 0x00ff;
    }else if(is_point_on_any_window_titlebar(mouse_x, mouse_y)) {
        mouse_color = 0x00ff00;
    }

    if(left_mouse) {
        Window_t* tmp = is_point_on_any_window_titlebar(mouse_x, mouse_y);
        if(tmp && tmp->with_title_bar) {
            if(point_in_rect(
                mouse_x,
                mouse_y,
                tmp->x + tmp->width - WINDOW_TITLEBAR_HEIGHT - 4,
                tmp->y - WINDOW_TITLEBAR_HEIGHT + 2,
                WINDOW_TITLEBAR_HEIGHT + 2,
                WINDOW_TITLEBAR_HEIGHT - 4
            ) && !dragging_mode) {
                if(tmp->on_close)
                    tmp->on_close(tmp);

                window_destroy(tmp);
                return;
            }else if(!dragging_mode){
                drag_window = tmp;
                dragging_mode = true;

                dragx = drag_window->x - mouse_x;
                dragy = drag_window->y - mouse_y;

                size_t sw = get_window_index(tmp);
                size_t mw = get_window_index(focused);

                if(sw < mw) {
//                    WINDOW(sw) = focused;
//                    WINDOW(mw) = tmp;

                    vector_swap(windows, sw, mw);
                }

                focused = tmp;
            }
        }else{
            Window_t* win = is_point_on_any_window(mouse_x, mouse_y);
            if(win && !dragging_mode && !click) {
                click = true;

                window_send_signal(win, WINDOW_CLICK, &(Coordinates_t){
                    mouse_x, mouse_y
                });
                qemu_log("End of click");
            }
        }
    }

    if(dragging_mode) {
        // FIXME: Need correct formula to correct window dragging
        drag_window->x = mouse_x + dragx;
        drag_window->y = mouse_y + dragy;
    }

    if(dragging_mode && (!left_mouse)) {
        drag_window = 0;
        dragging_mode = false;
    }

    if(click && (!left_mouse)) {
        click = false;
    }

	draw_filled_rectangle(mouse_x, mouse_y, 8, 8, mouse_color);
}

void gui_render_windows(vector_t* current_windows) {

    for(size_t i = 0, wnds = get_window_count(); i < wnds; i++){
//        qemu_warn("Rendering %x", WINDOW(i));
        gui_render_window(WINDOW(i));
    }
}

// Return exit status
void gui_render() {
    if(windows == 0) {
        draw_filled_rectangle(0, 0, getScreenWidth(), getScreenHeight(), 0x666666);
        draw_vga_str("No windows in the system.", 25, (getScreenWidth() - 25 * 8) / 2, (getScreenHeight() - 8) / 2, 0);
        punch();
        return;
    }

    // qemu_log("Reached render step");
    // qemu_log("Window count: %d", get_window_count());

    gui_render_windows(windows);

    gui_handle_mouse();
    punch();
}