#include <kernel.h>

void draw_line(int x0, int y0, int x1, int y1, int thickness, int color) {
    // Calculate the differences and absolute values of x and y coordinates
    int dx = ABS(x1 - x0);
    int dy = ABS(y1 - y0);
    
    // Determine the direction for incrementing x and y
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    
    // Calculate the initial error term
    int error = dx - dy;
    
    // Start drawing the line
    while (1) {
        // Set pixels for the line segment with the specified thickness
        for (int i = -thickness / 2; i <= thickness / 2; i++) {
            set_pixel(x0 + i, y0, color);  // Draw a horizontal line segment
        }
        
        // Check if we have reached the end point
        if (x0 == x1 && y0 == y1) {
            break;
        }
        
        int error2 = error * 2;
        
        // Adjust the coordinates based on the error term
        if (error2 > -dy) {
            error -= dy;
            x0 += sx;
        }
        
        if (error2 < dx) {
            error += dx;
            y0 += sy;
        }
    }
}


void draw_line_extern(char* buffer, size_t width, size_t height, int x0, int y0, int x1, int y1, int thickness, int color) {
    // Calculate the differences and absolute values of x and y coordinates
    int dx = ABS(x1 - x0);
    int dy = ABS(y1 - y0);
    
    // Determine the direction for incrementing x and y
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    
    // Calculate the initial error term
    int error = dx - dy;
    
    // Start drawing the line
    while (1) {
        // Set pixels for the line segment with the specified thickness
        for (int i = -thickness / 2; i <= thickness / 2; i++) {
            buffer_set_pixel4(buffer, width, height, x0 + i, y0, color);  // Draw a horizontal line segment
        }
        
        // Check if we have reached the end point
        if (x0 == x1 && y0 == y1) {
            break;
        }
        
        int error2 = error * 2;
        
        // Adjust the coordinates based on the error term
        if (error2 > -dy) {
            error -= dy;
            x0 += sx;
        }
        
        if (error2 < dx) {
            error += dx;
            y0 += sy;
        }
    }
}