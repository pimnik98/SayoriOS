#include "desktop/window.h"
#include "desktop/render.h"
#include "io/ports.h"
#include "mem/vmm.h"
#include "../lib/libvector/include/vector.h"

size_t window_global_id = 0;
size_t window_count = 0;

vector_t* windows = 0;

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
    win->widgets = vector_new();

    qemu_log("Bare window create ok");

    return win;
}

vector_t * get_window_list() {
    return windows;
}

size_t get_window_count() {
    return window_count;
}

void window_add_widget(Window_t* window, Widget_t* widget) {
    qemu_log("Widget count start: %d", window->widgets->size);

    vector_push_back(window->widgets, (size_t) widget);

    qemu_log("Widget count end: %d", window->widgets->size);
    qemu_log("Added widget %x to window %x", widget, window);
}

void window_remove_widget(Window_t* window, Widget_t* widget) {
    for (size_t i = 0; i < window->widgets->size; i++) {
        if(window->widgets->data[i]) {
            if((Widget_t*)window->widgets->data[i] == widget) {
                vector_erase_nth(window->widgets, i);
            }
        }
    }
}

void _window_add_to_list(Window_t* window) {
    if(!windows) {
        qemu_log("First time creating windows array!!!");
        windows = vector_new();
    }

    qemu_log("Expanding windows array to %d elements", window_count + 1);
    vector_push_back(windows, (size_t) window);

    window_count++;
}

void _window_remove_from_list(Window_t* window) {
    qemu_log("Window count: %d", window_count);
    qemu_log("Removing window with id: %d; Address: %x", window->id, window);
    for (size_t i = 0; i < window_count; i++) {
        if(WINDOW(i)) {
            if(WINDOW(i)->id == window->id) {
                qemu_log("Found that window at index: %d.", i);
                qemu_log("Total widgets in window: %d", window->widgets->size);
                for (size_t j = 0; j < window->widgets->size; j++) {
                    qemu_log("|- Removing widget: %x", window->widgets[j]);
                    qemu_log("|--- Destroyer at: %x", ((Widget_t *)(window->widgets->data[j]))->destroyer);
                    destroy_widget((Widget_t *) window->widgets->data[j]);
                }
                vector_destroy(window->widgets);

                window_count--;
                qemu_log("Now, %d windows in the system", window_count);

                vector_erase_nth(windows, i);

                if(window_count == 0) {
                    qemu_log("ZEROING WINDOWS ARRAY");
                    vector_destroy(windows);
                }

                qemu_log("Ok!!!");
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

// Returns true if window in list false otherwise
bool window_is_in_list(Window_t* win) {
    for(size_t i = 0; i < windows->size; i++) {
        if (WINDOW(i) == win) {
            return true;
        }
    }

    return false;
}

void window_destroy(Window_t* win) {
    qemu_log(" ========> Reached window_destroy()");
    if(!windows) {
        qemu_err("No array!");
        return;
    }
    if(!win) {
        qemu_log("Window address is 0, not destroying it");
        return;
    }

    if(!window_is_in_list(win)) {
        qemu_warn("Window not in window list so, please destroy it manually!");
        return;
    }

    if(win->state != NOT_INITIALIZED) {
        if(win->on_close)
            win->on_close(win);

        _window_remove_from_list(win);  // Reallocate is here
    } else {
        qemu_warn("Window is not initialized, not destroying it");
    }

    qemu_log("Freeing all its data at: %x", win);

    kfree(win);

    qemu_log("Ok");
}

void destroy_all_windows() {
    qemu_warn("DESTROYING ALL WINDOWS KNOWN TO WINDOW MANAGER!!!");
    while(window_count > 0) {
        qemu_log("%d windows remaining...", window_count);
        window_destroy(WINDOW(window_count - 1));
    }

    windows = 0;
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
        qemu_log("Widget count: %d", window->widgets->size);

        for (size_t i = 0; i < window->widgets->size; i++) {
            Widget_t* wgt = (Widget_t *) window->widgets->data[i];
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

// For debugging purposes only!!!
void log_window_manager_state() {
    qemu_log("===============[WINDOW MANAGER STATE]===================");
    qemu_log("Window next global ID to be specified to next window: %d", window_global_id);
    qemu_log("Window count: %d", window_count);
    qemu_log("Address of windows array: %x", windows);

    for(size_t i = 0; i < window_count; i++) {
        qemu_log("Window #%d (%x): ID: %d", i, windows[i], WINDOW(i)->id);
        if(WINDOW(i) != 0) {
            qemu_log("Widgets:");
            for (size_t j = 0; j < WINDOW(i)->widgets->size; j++) {
                qemu_log("|- Widget #%d: at %x", j, WINDOW(i)->widgets[j]);
            }
            
        }
    }

    qemu_log("========================================================");
}