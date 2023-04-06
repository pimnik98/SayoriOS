#include "gui/window.h"
#include "gui/basics.h"
#include "gui/render.h"
#include "gui/pointutils.h"
#include "io/tty.h"
#include "io/ports.h"
#include "io/imaging.h"
#include "drv/input/mouse.h"
#include "sys/memory.h"
#include "lib/string.h"

// getConfigFonts(2) - is height of current font

Window_t* active_window = 0;  // Window that should be placed over all windows (selected window) 
Window_t** current_windows = 0;

bool dragging_mode = false;
bool click = false;
Window_t* drag_window = 0;

void* cursor_data = 0;
size_t cursor_width = 0, cursor_height = 0;
DukeHeader_t cursor_meta;
bool cursor_ok = false;

Window_t* focused = 0;

char* cursor_default_path = "/var/cursor_normal.duke";

void do_load_cursor_from_duke_image() {
    qemu_log("DUKE2CURSOR: Loading cursor from %s", cursor_default_path);

    if(cursor_ok) return;  // Do not reallocate memory second time.

    char error = duke_get_image_metadata(cursor_default_path, &cursor_meta);
    if(error) {
        qemu_log("DUKE2CURSOR: Failed to load: %s", cursor_default_path);
        return;
    }

    cursor_data = kmalloc(cursor_meta.data_length);
    if(!cursor_data) {
        qemu_log("DUKE2CURSOR: Failed to allocate %d bytes for DUKE image container!", cursor_meta.data_length);
        return;
    }

    duke_get_image_data(cursor_default_path, cursor_meta, cursor_data);
    cursor_ok = true;
}

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
    Widget_t** widgets = window->widgets;

    // qemu_log("Widget list: %x", widgets);
    // qemu_log("Widget count: %d", window->widget_count);

    /*
    for (size_t i = 0; i < window->widget_count; i++) {
        // Create copy of Widget and correct to 'stick' to window!
        Widget_t* wgt = widgets[i];
        // qemu_log("Widget original: %x", wgt);

        Widget_t* window_linked_wgt = kcalloc(1, sizeof(Widget_t));  // Fails with kcalloc!!!
        // qemu_log("Widget copy: %x", window_linked_wgt);

        memcpy(window_linked_wgt, wgt, sizeof(Widget_t));
        // qemu_log("Copied");

        window_linked_wgt->x += window->x;
        window_linked_wgt->y += window->y;

        // qemu_log("Rendering");

        window_linked_wgt->renderer(window_linked_wgt, window);

        kfree(window_linked_wgt);
    }
    */
    
    // /*
    for (size_t i = 0; i < window->widget_count; i++) {
        Widget_t* wgt = widgets[i];

        wgt->x += window->x;
        wgt->y += window->y;

        wgt->renderer(wgt, window);

        wgt->x -= window->x;
        wgt->y -= window->y;
    }
    // */
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
            window->x + window->width - WINDOW_TITLEBAR_HEIGHT + 2,
            window->y - WINDOW_TITLEBAR_HEIGHT + 5,
            0
        );
    }

    gui_render_widgets(window);
}

Window_t* is_point_on_any_window_titlebar(ssize_t x, ssize_t y) {
    for (size_t i = 0; i < get_window_count(); i++) {
        if(current_windows[i]->with_title_bar && point_in_rect(
            x,
            y,
            current_windows[i]->x,
            current_windows[i]->y - WINDOW_TITLEBAR_HEIGHT,
            current_windows[i]->width,
            WINDOW_TITLEBAR_HEIGHT
            )) {
                return current_windows[i];
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
            current_windows[i]->x,
            current_windows[i]->y,
            current_windows[i]->width,
            current_windows[i]->height
            )) {
                // qemu_log("It's: %d", current_windows[i]->id);
                return current_windows[i];
            }
    }
    return 0;
}

size_t get_window_index(Window_t* win) {
    for(size_t i = 0, wnds = get_window_count(); i < wnds; i++){
        if(get_window_list()[i] == win) {
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
        if((tmp != 0) && tmp->with_title_bar) {
            if(point_in_rect(
                mouse_x,
                mouse_y,
                tmp->x + tmp->width - WINDOW_TITLEBAR_HEIGHT - 4,
                tmp->y - WINDOW_TITLEBAR_HEIGHT + 2,
                WINDOW_TITLEBAR_HEIGHT + 2,
                WINDOW_TITLEBAR_HEIGHT - 4
            ) && !dragging_mode) {
                window_destroy(tmp);
                return;
            }else{
                drag_window = tmp;
                dragging_mode = true;

                size_t sw = get_window_index(tmp);
                size_t mw = get_window_index(focused);

                if(sw < mw) {
                    get_window_list()[sw] = focused;
                    get_window_list()[mw] = tmp;
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
        drag_window->x = mouse_x - (drag_window->width/2);
        drag_window->y = mouse_y + (WINDOW_TITLEBAR_HEIGHT/2);
    }

    if(dragging_mode && (!left_mouse)) {
        drag_window = 0;
        dragging_mode = false;
    }

    if(click && (!left_mouse)) {
        click = false;
    }

    if(cursor_ok) {
        duke_rawdraw(cursor_data, &cursor_meta, mouse_x, mouse_y);
    }else{
        draw_filled_rectangle(mouse_x, mouse_y, 8, 8, mouse_color);
    }
}

void gui_render_windows(Window_t** current_windows) {
    for(size_t i = 0, wnds = get_window_count(); i < wnds; i++){
        // qemu_log("Window %d: W: %d; H: %d", i, current_windows[i]->width, current_windows[i]->height);
        gui_render_window(current_windows[i]);
    }
}

// Return exit status
void gui_render() {
    current_windows = get_window_list();

    if(current_windows == 0) {
        draw_filled_rectangle(0, 0, getWidthScreen(), getHeightScreen(), 0x666666);
        draw_vga_str("No windows in the system.", 25, (getWidthScreen() - 25 * 8) / 2, (getHeightScreen()- 8) / 2, 0);
        punch();
        return;
    }

    // qemu_log("Reached render step");
    // qemu_log("Window count: %d", get_window_count());

    gui_render_windows(current_windows);

    gui_handle_mouse();
    punch();
}