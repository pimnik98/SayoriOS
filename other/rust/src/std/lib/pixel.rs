// kernel/src/lib/pixel.c wrapper for Rust

extern "C" {
    fn drawRect(x: u32, y: u32, w: u32, h: u32, color: u32);
    fn drawRectLine(x: u32, y: u32, w: u32, h: u32, color: u32, color2: u32, c: i32);
    fn drawRectBorder(x: u32, y: u32, w: u32, h: u32, color: u32);
    fn drawHorizontalLine(x1: u32, x2: u32, y: u32, color: u32);
    fn drawVerticalLine(y1: u32, y2: u32, x: u32, color: u32);
    fn drawCirclePoints(cx: u32, cy: u32, x: u32, y: u32, color: u32);
    fn drawCircle(cx: u32, cy: u32, radius: u32, color: u32);
    fn drawFilledCircle(x0: u32, y0: u32, radius: u32, color: u32);
    fn drawFilledRectBorder(x0: u32, y0: u32, radius: u32, w: u32, mode: u32, color: u32);
    fn drawRoundedSquare(x: u32, y: u32, size: u32, radius: u32, fill_color: u32, border_color: u32);
    fn drawRoundedRectangle(x: u32, y: u32, width: u32, height: u32, radius: u32, fill_color: u32, border_color: u32);
}

#[derive(Copy, Clone)]
pub struct Color {
    pub r: u32,
    pub g: u32,
    pub b: u32
}

impl Color {
    pub fn as_hex(&self) -> u32 {
        let r = self.r << 16;
        let g = self.g << 8;
        let b = self.b;

        r | g | b 
    }
}

pub fn draw_rect(x: u32, y: u32, w: u32, h: u32, color: Color) {
    unsafe { drawRect(x, y, w, h, color.as_hex()); }
}

pub fn draw_rect_line(x: u32, y: u32, w: u32, h: u32, color: Color, color2: Color, c: i32) {
    unsafe {
        drawRectLine(x, y, w, h, color.as_hex(), color2.as_hex(), c);
    }
}

pub fn draw_rect_border(x: u32, y: u32, w: u32, h: u32, color: Color) {
    unsafe { drawRectBorder(x, y, w, h, color.as_hex()); }
}

pub fn draw_horizontal_line(x1: u32, x2: u32, y: u32, color: Color) {
    unsafe { drawHorizontalLine(x1, x2, y, color.as_hex()) }
}

pub fn draw_vertical_line(y1: u32, y2: u32, x: u32, color: Color) {
    unsafe { drawVerticalLine(y1, y2, x, color.as_hex()); }
}

pub fn draw_circle_points(cx: u32, cy: u32, x: u32, y: u32, color: Color) {
    unsafe { drawCirclePoints(cx, cy, x, y, color.as_hex()) }
}

pub fn draw_circle(cx: u32, cy: u32, radius: u32, color: Color) {
    unsafe { drawCircle(cx, cy, radius, color.as_hex()); }
}

pub fn draw_filled_circle(x0: u32, y0: u32, radius: u32, color: Color) {
    unsafe { drawFilledCircle(x0, y0, radius, color.as_hex()); }
}

pub fn draw_filled_rect_border(x0: u32, y0: u32, radius: u32, w: u32, mode: u32, color: Color) {
    unsafe { drawFilledRectBorder(x0, y0, radius, w, mode, color.as_hex()); }
}

pub fn draw_rounded_square(x: u32, y: u32, size: u32, radius: u32, fill_color: Color, border_color: Color) {
    unsafe { drawRoundedSquare(x, y, size, radius, fill_color.as_hex(), border_color.as_hex()) }
}

pub fn draw_rounded_rectangle(x: u32, y: u32, width: u32, height: u32, radius: u32, fill_color: Color, border_color: Color) {
    unsafe { drawRoundedRectangle(x, y, width, height, radius, fill_color.as_hex(), border_color.as_hex()); }
}