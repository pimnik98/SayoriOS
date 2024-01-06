#include "desktop/render.h"
#include "desktop/widget_button.h"
#include "desktop/widget_label.h"
#include "io/serial_port.h"
#include "lib/stdlib.h"
#include "drv/cmos.h"
#include "drv/input/keyboard.h"
#include "io/ports.h"
#include "mem/vmm.h"
#include "lib/sprintf.h"
#include "desktop/widget_image.h"
#include "sys/scheduler.h"
#include "desktop/widget_progress.h"
#include "sys/timer.h"

extern Window_t* focused;

void eki_start();

char time_for_label[] = "--:--:--";
char membuf[16] = {0};
char label_for_memory[32] = {0};
char label_for_fps[8] = {0};

void make_time_string(char* out) {
    sayori_time_t time = get_time();

    uint8_t h = time.hours;
    uint8_t m = time.minutes;
    uint8_t s = time.seconds;

    sprintf(out, "%02d:%02d:%02d", h, m, s);
}

void shutdown_system_activity_real() {
    Window_t* progwin = window_new("Shutting down...");

    progwin->x = 20;
    progwin->y = 20;

    progwin->width = 500;
    progwin->height = 160;

    Widget_t* progress = new_widget_progress();

    progress->x = 10;
    progress->y = 10;

    progress->width = 400;
    progress->height = 30;

    ((Widget_Progress_t*)progress->custom_widget_data)->current = 0;

    window_add_widget(progwin, progress);

    Widget_t* lab = new_widget_label("Getting ready to shutdown...", 10, 70, 0x00ffff);

    window_add_widget(progwin, lab);

    progwin->state = DISPLAYING;

    size_t tst = timestamp();

    while(1) {
//        if(timestamp() - tst > 100) {
//            tst = timestamp();
//
//            ((Widget_Progress_t*)progress->custom_widget_data)->current++;
//        }
        if(timestamp() - tst > 11000) {
            break;
        } else if(timestamp() - tst > 9000) {
            ((Widget_Progress_t *) progress->custom_widget_data)->current = 90;
            ((Widget_Label_t*)lab->custom_widget_data)->label = "Destroying FS objects...";
        } else if(timestamp() - tst > 5000) {
            ((Widget_Progress_t*)progress->custom_widget_data)->current = 50;
            ((Widget_Label_t*)lab->custom_widget_data)->label = "Synchronizing data to disks...";
        } else if(timestamp() - tst > 3500) {
            ((Widget_Progress_t*)progress->custom_widget_data)->current = 40;
            ((Widget_Label_t*)lab->custom_widget_data)->label = "Flushing logs...";
        } else if(timestamp() - tst > 2500) {
            ((Widget_Progress_t*)progress->custom_widget_data)->current = 20;
            ((Widget_Label_t*)lab->custom_widget_data)->label = "Notifying services...";
        } else if(timestamp() - tst > 1000) {
            ((Widget_Progress_t*)progress->custom_widget_data)->current = 10;
            ((Widget_Label_t*)lab->custom_widget_data)->label = "Destroying FS objects...";
        }
    }

    shutdown();
}

void shutdown_system_activity() {
    thread_create(
        get_current_proc(),
        shutdown_system_activity_real,
        512,
        true,
        false
    );
}

void parallel_desktop_start() {
	size_t frames = 0;

    qemu_log("Reached init...");    
    set_cursor_enabled(false);
    keyboardctl(KEYBOARD_ECHO, false);

    log_window_manager_state();

    // ROOT WINDOW

    Window_t* root_window = window_new(0);
    root_window->x = 0;
    root_window->y = 0;
    root_window->width = getScreenWidth();
    root_window->height = getScreenHeight();
    root_window->with_title_bar = false;
    root_window->closable = false;
    root_window->canvas_bgcolor = 0x404040;

    qemu_log("Root window! at %x", root_window);

    Widget_t* wallpaper = new_widget_image("R:\\Sayori\\bg.tga");
    window_add_widget(root_window, wallpaper);

    qemu_log("Wallpaper for it...");

    // TASKBAR

    Window_t* taskbar = window_new("taskbar");
    taskbar->x = 0;
    taskbar->height = 30;
    taskbar->y = getScreenHeight() - taskbar->height;
    taskbar->width = getScreenWidth();

    taskbar->with_title_bar = false;
    taskbar->closable = false;
    taskbar->canvas_bgcolor = 0x777777;


    // TASKBAR: TIME

    qemu_log("Creating time label");

    Widget_t* time_label = new_widget_label(time_for_label,
        taskbar->width - 8 * 8 - 8,
        (taskbar->height - 16) / 2,
        0x000000
    );

    Widget_t* memory_label = new_widget_label(label_for_memory,
        taskbar->width - 8 * strlen(label_for_memory) - 256, 
        (taskbar->height - 16) / 2,
        0x000000
    );
    
    uint8_t seconds_old = get_time().seconds;

    Widget_t* fps_label = new_widget_label(label_for_fps,
        32, 
        (taskbar->height - 16) / 2,
        0x000000
    );

    // // ((Widget_Label_t*)(widget->custom_widget_data))->color = 0x404040;

    window_add_widget(taskbar, time_label);
    window_add_widget(taskbar, memory_label);
    window_add_widget(taskbar, fps_label);

    // TEST WINDOW

    qemu_log("Creating test window");

    Window_t* window = window_new("Untitled");
    window->x = 100;
    window->y = 100;
    window->width = 300;
    window->height = 300;

    focused = window;

    qemu_log("Untitled window");

    Widget_t* hello_button = new_widget_button("Create test window", 0x00ff00, 0x000000);
    hello_button->x = 50;
    hello_button->y = 50;
    hello_button->width  += 20;
    hello_button->height += 15;
    hello_button->on_click = eki_start;

    qemu_log("Button for it...");

    window_add_widget(window, hello_button);

    Widget_t* sh_btn = new_widget_button("Shutdown system", 0xff0000, 0x000000);
    sh_btn->x = 50;
    sh_btn->y = 80;
    sh_btn->width  += 20;
    sh_btn->height += 15;
    sh_btn->on_click = shutdown_system_activity;

    qemu_log("Button for it...");

    window_add_widget(window, sh_btn);

    root_window->state = DISPLAYING;
    taskbar->state = DISPLAYING;
    window->state = DISPLAYING;

    qemu_log("Start...");

    for(;;) {
        if(getCharRaw() == 129) {
            gui_restore();
            break;
        }

        make_time_string(time_for_label);
        gui_render();

        sprintf(label_for_memory, "%d kB used", system_heap.used_memory / 1024);

        frames++;
		if(get_time().seconds != seconds_old) {
            sprintf(label_for_fps, "%d FPS", frames);

			frames = 0;
            seconds_old = get_time().seconds;
		}
    }

    qemu_log("Loop exit");

    window_destroy(window);
    qemu_log("Destroyed window");
    window_destroy(taskbar);
    qemu_log("Destroyed taskbar");
    window_destroy(root_window);
    qemu_log("Destroyed root_window");

    set_cursor_enabled(true);
    
    qemu_log("Exit successfully!!!");

    clean_tty_screen();

    tty_printf("Memory allocation info written to COM1 (debug) port!!!");
    log_window_manager_state();

    keyboardctl(KEYBOARD_ECHO, true);
}
