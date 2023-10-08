// Måla v0.1 (Swedish - Draw) (read as Mola) by NDRAEY (c) 2023

#include <kernel.h>

#define BUFSIZE(width, height) (width * height * 4)
#define STATUSBAR_HEIGHT 32

uint8_t* buffer = 0;

size_t canvas_width = 0;
size_t canvas_height = 0;

size_t cursor_pos_x = 0;
size_t cursor_pos_y = 0;

size_t current_color = 0x000000;
bool is_click = false;

size_t buffer_size = 0;
size_t brush_size = 3;

size_t old_x = 0;
size_t old_y = 0;

bool change_coords = false;

#define COLORS 8

typedef struct {
    size_t x;
    size_t y;
    size_t width;
    size_t height;
    size_t color;
} ColorZone_t;

ColorZone_t colors[COLORS] = {
    {400, 4, 25, 25, 0xFF0000},
    {430, 4, 25, 25, 0x00FF00},
    {460, 4, 25, 25, 0x0000FF},
    {490, 4, 25, 25, 0x000000},
    {520, 4, 25, 25, 0xFFFF00},
    {550, 4, 25, 25, 0x00FFFF},
    {580, 4, 25, 25, 0xFF00FF},
    {610, 4, 25, 25, 0xFFFFFF}
};

char text_buffer[16] = {0};

void mala_init() {
    canvas_width = getScreenWidth();
    canvas_height = getScreenHeight() - STATUSBAR_HEIGHT;

    buffer_size = BUFSIZE(canvas_width, canvas_height);

    buffer = kmalloc(buffer_size);

    memset(buffer, 0xFF, buffer_size);
}

void mala_flush() {
    memcpy((char*)getFrameBufferAddr() + BUFSIZE(canvas_width, STATUSBAR_HEIGHT), buffer, buffer_size);


    for(int i = 0; i < COLORS; i++) {
        // Border
        
        drawRect(
            colors[i].x - 2,
            colors[i].y - 2,
            colors[i].width + 4,
            colors[i].height + 4,
            0
        );
    
        // Color
        drawRect(
            colors[i].x,
            colors[i].y,
            colors[i].width,
            colors[i].height,
            colors[i].color
        );
    }

    drawRect(cursor_pos_x, cursor_pos_y, 16, 16, current_color);

    punch();
}

void mala_exit() {
    kfree(buffer);

    set_cursor_enabled(true);

    clean_tty_screen();
}

bool check_colors() {
    for(int i = 0; i < COLORS; i++) {
        if(point_in_rect(
            cursor_pos_x,
            cursor_pos_y,
            colors[i].x,
            colors[i].y,
            colors[i].width,
            colors[i].height
        ) && is_click) {
            current_color = colors[i].color;
            return true;
        }
    }
    return false;
}

void mala_control() {
    drawRect(0, 0, canvas_width, STATUSBAR_HEIGHT, 0x666666);

    draw_vga_str("Mala v0.2", 9, 16, 10, 0);
    
    itoh(current_color, text_buffer);
    drawRect(100, 0, 2, STATUSBAR_HEIGHT, 0);
    draw_vga_str(text_buffer, 6, 108, 10, 0);

    memset(text_buffer, 0, 16);
    itoa(cursor_pos_x, text_buffer);
    drawRect(172, 0, 2, STATUSBAR_HEIGHT, 0);
    draw_vga_str(text_buffer, 4, 180, 10, 0);

    memset(text_buffer, 0, 16);
    itoa(cursor_pos_y, text_buffer);
    draw_vga_str(text_buffer, 4, 212, 10, 0);

    cursor_pos_x = mouse_get_x();
    cursor_pos_y = mouse_get_y();

    is_click = (bool)mouse_get_b1();

    bool in_canvas = point_in_rect(cursor_pos_x, cursor_pos_y, 0, STATUSBAR_HEIGHT, canvas_width - 1, canvas_height - 1);

    if(check_colors())
        return;

    if(is_click) {
        draw_line_extern(
            buffer,
            canvas_width,
            canvas_height,
            old_x,
            old_y - STATUSBAR_HEIGHT,
            cursor_pos_x,
            cursor_pos_y - STATUSBAR_HEIGHT,
            brush_size,
            current_color
        );
    }

    if(in_canvas) {
        old_x = cursor_pos_x;
        old_y = cursor_pos_y;
    }
}

void mala_draw() {
    // Disable TTY cursor

    mala_init();

    set_cursor_enabled(false);

    while(1) {
        if(getCharRaw() == 1) {
            mala_exit();
            break;
        }

        if(getCharRaw() == 48) {
            memset(buffer, 0xFF, buffer_size);
        }

        mala_control();

        mala_flush();
    }
}