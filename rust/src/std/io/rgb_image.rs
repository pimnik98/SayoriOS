// kernel/src/io/rgb_image.c обертка для Rust

extern "C" {
    fn draw_rgb_image(data: *const u8, width: isize, height: isize, bpp: isize, sx: u32, sy: u32);
}

#[derive(Copy, Clone)]
pub struct Image<'a> {
    data: &'a [u8],
    x: u32,
    y: u32,
    width: isize,
    height: isize,
    bpp: isize,
}

impl<'a> Image<'a> {

    /// Создает новый экземпляр структуры Image.
    ///
    ///
    /// * data - Данные изображения в формате RGB.
    /// * x - Координата X для отрисовки изображения.
    /// * y - Координата Y для отрисовки изображения.
    /// * width - Ширина изображения.
    /// * height - Высота изображения.
    /// * bpp - Количество байт на пиксель.
    ///
    ///
    /// Возвращает новый экземпляр структуры Image.
    ///
    /// # Пример
    ///
    /// 
    /// let data: &[u8] = &[255, 0, 0, 0, 0, 255, 0, 0, 0, 0, 255, 0];
    /// let image = Image::new(data, 0, 0, 3, 1, 32);
    /// image.draw();
    /// 

    pub fn new(data: &'a [u8], x: u32, y: u32,
        width: isize, height: isize, bpp: isize) -> Self {
            assert!(width >= 0 && height >= 0 && bpp >= 0);
            Self { data, x, y, width, height, bpp }
    }

    /// Отрисовывает изображение.
    pub fn draw(&self) {
        unsafe {
            draw_rgb_image(self.data.as_ptr(),
                self.width, self.height, self.bpp, self.x, self.y);
        }
    }
}