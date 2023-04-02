#include "gui/render.h"
#include "gui/widget_image.h"
#include "gui/widget_button.h"
#include "gui/widget_label.h"
#include "io/port_io.h"
#include "lib/stdlib.h"

extern Window_t* focused;

void eki_start();

char time_for_label[] = "--:--:--";
char membuf[16] = {0};
char label_for_memory[32] = {0};
char label_for_fps[8] = {0};

void make_time() {
    sayori_time_t time = get_time();
    
    uint8_t h = time.hours;
    uint8_t m = time.minutes;
    uint8_t s = time.seconds;

    if(h < 10) {
        time_for_label[0] = '0';
        time_for_label[1] = ('0' + h);
    }else{
        time_for_label[0] = '0' + (h / 10);
        time_for_label[1] = '0' + (h % 10);
    }

    if(m < 10) {
        time_for_label[3] = '0';
        time_for_label[4] = ('0' + m);
    }else{
        time_for_label[3] = '0' + (m / 10);
        time_for_label[4] = '0' + (m % 10);
    }

    if(s < 10) {
        time_for_label[6] = '0';
        time_for_label[7] = ('0' + s);
    }else{
        time_for_label[6] = '0' + (s / 10);
        time_for_label[7] = '0' + (s % 10);
    }
}

void parallel_desktop_start() {
	size_t frames = 0;

    qemu_log("Reached init...");    
    mouse_set_show_system_cursor(false);
    set_cursor_enabled(false);

    log_window_manager_state();
    
    do_load_cursor_from_duke_image();

    // ROOT WINDOW

    Window_t* root_window = window_new(0);
    root_window->x = 0;
    root_window->y = 0;
    root_window->width = getWidthScreen();
    root_window->height = getHeightScreen();
    root_window->with_title_bar = false;
    root_window->closable = false;
    root_window->canvas_bgcolor = 0x404040;

    qemu_log("Root window! at %x", root_window);

    Widget_t* wallpaper = new_widget_image("/var/Jump.duke");
    qemu_log("Widget Image Info: X: %d; Y: %d W: %d; H: %d",
        wallpaper->x,
        wallpaper->y,
        wallpaper->width,
        wallpaper->height
    );
    window_add_widget(root_window, wallpaper);

    qemu_log("Wallpaper for it...");

    // TASKBAR

    Window_t* taskbar = window_new("titlebar");
    taskbar->x = 0;
    taskbar->height = 30;
    taskbar->y = getHeightScreen() - taskbar->height;
    taskbar->width = getWidthScreen();

    taskbar->with_title_bar = false;
    taskbar->closable = false;
    taskbar->canvas_bgcolor = 0x777777;


    // TASKBAR: TIME

    qemu_log("Creating time label");

    Widget_t* time_label = new_widget_label(time_for_label,
        taskbar->width - 8 * strlen(time_for_label) - 8, 
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

    root_window->state = DISPLAYING;
    taskbar->state = DISPLAYING;
    window->state = DISPLAYING;

    qemu_log("Start...");

    for(;;) {
        if(getCharRaw() == 129) {
            gui_restore();
            break;
        }
        make_time();
        gui_render();

        itoa(memory_get_used_kernel()/1024, membuf);
        strcpy(label_for_memory, membuf);
        strcat(label_for_memory, " kBytes used");

        frames++;
		if(get_time().seconds != seconds_old) {
			memset(membuf, 0, 16);
			
	        itoa(frames, membuf);
    	    strcpy(label_for_fps, membuf);
        	strcat(label_for_fps, " FPS");
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
    mouse_set_show_system_cursor(true);

    qemu_log("Exit successfully!!!");

    print_allocated_map();
    tty_printf("Memory allocation info written to COM1 (debug) port!!!");
    log_window_manager_state();
}
