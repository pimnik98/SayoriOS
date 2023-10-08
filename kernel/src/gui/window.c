#include <kernel.h>
#include "gui/window.h"
#include "gui/render.h"

size_t window_global_id = 0;
size_t window_count = 0;
Window_t** windows = 0;

Window_t* _window_new(char* title) {
    Window_t* win = kcalloc(1, sizeof(Window_t));
    win->id = window_global_id++;
    win->width = 10;
    win->height = 10;
    win->x = 10;
    win->y = 10;
    win->title = title;
    win->state = NOT_INITIALIZED;
    win->closable = true;
    win->minimizable = true;
    win->with_title_bar = true;
    win->canvas_bgcolor = WINDOW_CANVAS_COLOR;
    win->widget_count = 0;
    win->widgets = 0;
    qemu_log("Bare window create ok");
    return win;
}

Window_t** get_window_list() {
    return windows;
}

size_t get_window_count() {
    return window_count;
}

void window_add_widget(Window_t* window, Widget_t* widget) {
    qemu_log("Widget count start: %d", window->widget_count);
    
    if(!window->widgets) {
        window->widgets = kcalloc(sizeof(Widget_t*), 1);
    }else{
        window->widgets = krealloc(window->widgets, sizeof(Widget_t*) * (window->widget_count + 1));
    }

    window->widgets[window->widget_count] = widget;
    window->widget_count++;

    qemu_log("Widget count end: %d", window->widget_count);
    qemu_log("Added widget %x to window %x", widget, window);
}

void window_remove_widget(Window_t* window, Widget_t* widget) {
    for (size_t i = 0; i < window->widget_count; i++) {
        if(window->widgets[i] != 0) {
            if(window->widgets[i] == widget) {
                window->widget_count--;

                while(i < window->widget_count) {
                    qemu_log("WIDGET ARRAY SHIFT: %x (POS: %d) = %x (POS: %d)", windows[i], i, windows[i+1], i+1);
                    window->widgets[i] = window->widgets[i+1];
                    i++;
                }
            }
        }
    }
}

void _window_add_to_list(Window_t* window) {
    if(!windows) {
        qemu_log("First time creating windows array!!!");
        windows = kcalloc(sizeof(Window_t*), 1);
    }else{
        qemu_log("Expanding windows array to %d elements", window_count+1);
        windows = krealloc(windows, sizeof(Window_t*) * (window_count+1));
    }

    windows[window_count++] = window;
    // window_count++;
}

void _window_remove_from_list(Window_t* window) {
    qemu_log("Window count: %d", window_count);
    qemu_log("Removing window with id: %d; Address: %x", window->id, window);
    for (size_t i = 0; i < window_count; i++) {
        if(windows[i]) {
            if(windows[i]->id == window->id) {
                qemu_log("Found that window at index: %d.", i);
                // TODO: Remove all widgets before freeing.
                qemu_log("Total widgets in window: %d", window->widget_count);
                for (size_t j = 0; j < window->widget_count; j++) {
                    qemu_log("|- Removing widget: %x", window->widgets[j]);
                    qemu_log("|--- Destroyer at: %x", window->widgets[j]->destroyer);
                    //window->widgets[j]->destroyer(window->widgets[j]);
                    destroy_widget(window->widgets[j]);
                }
                kfree(window->widgets);

                qemu_log("Shifting array");

                window_count--;
                qemu_log("Now, %d windows in the system", window_count);
                // while(windows[i+1] != 0) {
                while(i < window_count) {
                    qemu_log("%x (POS: %d) = %x (POS: %d)", windows[i], i, windows[i+1], i+1);
                    windows[i] = windows[i+1];
                    i++;
                }
                // windows[i+1] = 0;

                if(window_count > 0) {
                    qemu_log("Shrinking windows array to %d elements", window_count);
                    windows = krealloc(windows, sizeof(Window_t*) * (window_count));
                }else{
                    qemu_log("ZEROING WINDOWS ARRAY");
                    kfree(windows);
                    windows = 0;
                }

                qemu_log("Ok!!!");
                window_global_id--;
                break;
            }
        }
    }
}

Window_t* window_new(char* title) {
    Window_t* instance = _window_new(title);  // Allocate is here
    instance->state = HIDDEN;

    qemu_log("Created bare window");
    _window_add_to_list(instance);  // Allocate is here (many)

    return instance;
}

void window_destroy(Window_t* win) {
    qemu_log("Reached window_destroy()");
    if(!win) {
        qemu_log("Window address is 0, not destroying it");
        return;
    }
    if(win->state != NOT_INITIALIZED) {
        _window_remove_from_list(win);  // Reallocate is here
    }

    qemu_log("Freeing all its data at: %x", win);

    kfree(win);

    qemu_log("Ok");
}

void destroy_all_windows() {
    qemu_log("DESTROYING ALL WINDOWS KNOWN TO WINDOW MANAGER!!!");
    while(window_count > 0) {
        qemu_log("%d windows remaining...", window_count);
        window_destroy(windows[window_count-1]);
    }
    qemu_log("DESTROYED ALL WINDOWS!");
}

void window_send_signal(Window_t* window, WindowSignal_t signal, void* data) {
    qemu_log("Got signal on window (id %d): %s (data at %x)", window->id, 
        (signal == WINDOW_CLOSE ? 
            "WINDOW_CLOSE":
            (signal == WINDOW_CLICK ? 
                "WINDOW_CLICK":
                (signal == WINDOW_MINIMIZE ? 
                    "WINDOW_MINIMIZE":"UNKNOWN"
                )
            )
        ), data
    );

    if(signal == WINDOW_CLICK) {
        Coordinates_t* coords = (Coordinates_t*)data;

        coords->x -= window->x;
        coords->y -= window->y;

        qemu_log("Click on: X: %d; Y: %d", coords->x, coords->y);
        qemu_log("Widget count: %d", window->widget_count);

        for (size_t i = 0; i < window->widget_count; i++) {
            Widget_t* wgt = window->widgets[i];
            // qemu_log("Getting over widget: at %x", wgt);
            // qemu_log("X: %d; Y: %d; W: %d; H: %d;", wgt->x, wgt->y, wgt->width, wgt->height);

            if(wgt && point_in_rect(
                coords->x,
                coords->y,
                wgt->x,
                wgt->y,
				(ssize_t)wgt->width,
				(ssize_t)wgt->height
            )) {
                widget_notify(window, wgt, WIDGET_CLICK, coords);
                qemu_log("Notified for click");
                break;
            }
        }
    }

    qemu_log("Func end");
}

void window_system_init() {

}

// For debugging purposes only!!!
void log_window_manager_state() {
    qemu_log("===============[WINDOW MANAGER STATE]===================");
    qemu_log("Window next global ID to be specified to next window: %d", window_global_id);
    qemu_log("Window count: %d", window_count);
    qemu_log("Address of windows array: %d", windows);

    for(size_t i = 0; i < window_count; i++) {
        qemu_log("Window #%d (%x): ID: %d", i, windows[i], windows[i]->id);
        if(windows[i] != 0) {
            qemu_log("Widgets:");
            for (size_t j = 0; j < windows[i]->widget_count; j++) {
                qemu_log("|- Widget #%d: at %x", j, windows[i]->widgets[j]);
            }
            
        }
    }

    qemu_log("========================================================");
}