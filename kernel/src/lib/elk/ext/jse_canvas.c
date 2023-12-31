/// JavaScript Engine - Поддержка библиотек
#include "portability.h"
#include <io/ports.h>
#include "io/tty.h"
#include "lib/stdio.h"

#include "../elk_config.h"
#include "../elk.h"

/// Планы на canvas, список функций и примеров можно глянуть по этой ссылке
/// https://msiter.ru/references/canvas-reference

/// Пиксельные манипуляции (0 из 6)
/// [-] canvas.data                     | Возвращает объект, содержащий данные изображения заданного объекта ImageData
/// [+] canvas.height                   | Возвращает высоту объекта ImageData
/// [+] canvas.width                    | Возвращает ширину объекта ImageData
/// [-] canvas.createImageData()        | Создает новый, пустой объект ImageData
/// [-] canvas.getImageData()           | Возвращает объект ImageData, который копирует пиксельные данные заданной прямоугольной области холста
/// [-] canvas.putImageData()           | Помещает данные изображения (из заданного объекта ImageData) обратно в элемент <canvas>

/// Цвета, стили, тени (1 из 10)
/// [+] canvas.fillStyle                | Устанавливает/возвращает цвет, градиент или шаблон, используемый для заливки графического объекта
/// [-] canvas.shadowBlur               | Устанавливает/возвращает уровень размытости для теней
/// [-] canvas.shadowColor              | Устанавливает/возвращает цвет для теней
/// [-] canvas.shadowOffsetX            | Устанавливает/возвращает горизонтальное расстояние тени от фигуры
/// [-] canvas.shadowOffsetY            | Устанавливает/возвращает вертикальное расстояние тени от фигуры
/// [-] canvas.strokeStyle              | Устанавливает/возвращает цвет, градиент или шаблон, используемый для обводки фигуры
/// [-] canvas.addColorStop()           | Определяет цвета и позицию остановки в объекте градиента
/// [-] canvas.createLinearGradient()   | Создает линейный градиент (для использования с содержимым элемента <canvas>)
/// [-] canvas.createPattern()          | Размножает заданный элемент в заданном направлении
/// [-] canvas.createRadialGradient()   | Создает радиальный/круговой градиент (для использования на содержимом элемента <canvas>)

/// Текст (0 из 6)
/// [-] canvas.font                     | Устанавливает/возвращает свойства шрифта для текстового содержимого
/// [-] canvas.textAlign                | Устанавливает/возвращает выравнивание для текстового содержимого
/// [-] canvas.textBaseline             | Устанавливает/возвращает базовую линию, используемую при выводе текста
/// [-] canvas.fillText()               | Рисует текст с заливкой
/// [-] canvas.measureText()            | Возвращает объект, содержащий ширину заданного текста
/// [-] canvas.strokeText()             | Рисует текст без заливки

/// Компоновка (0 из 2)
/// [-] canvas.globalAlpha              | Устанавливает/возвращает текущее значение прозрачности или альфа-канала графического объекта
/// [-] canvas.globalCompositeOperation	| Устанавливает/возвращает то, как исходное (новое) изображение нарисовано на целевом (существующем) изображении

/// Стили линий (1 из 4)
/// [-] canvas.lineCap                  | Устанавливает/возвращает стиль концов нарисованной линии
/// [-] canvas.lineJoin                 | Устанавливает/возвращает тип угла, созданного пересечением двух линий
/// [+] canvas.lineWidth                | Устанавливает/возвращает ширину текущей линии
/// [-] canvas.miterLimit               | Устанавливает/возвращает максимальную длину среза

/// Контуры (0 из 12)
/// [-] canvas.arc()                    | Создает дугу/кривую (используется для создания окружностей или их части)
/// [-] canvas.arcTo()                  | Создает дугу/кривую между двумя касательными
/// [-] canvas.beginPath()              | Начинает контур или сбрасывает текущий контур
/// [-] canvas.bezierCurveTo()          | Создает кубическую кривую Безье
/// [-] canvas.clip()                   | Обрезает область любой формы и размера, находящуюся вне указанного контура
/// [-] canvas.closePath()              | Замыкает контур соединяя последнюю точку с первой
/// [-] canvas.fill()                   | Делает заливку текущей фигуры (контура)
/// [-] canvas.isPointInPath()          | Возвращает значение true, если заданная точка находится внутри текущего контура, в обратном случае возвращается значение false
/// [-] canvas.lineTo()                 | Добавляет новую точку контура и создает линию к этой точке от последней заданной точки
/// [-] canvas.moveTo()                 | Передвигает точку контура в заданные координаты не рисуя линию
/// [-] canvas.quadraticCurveTo()       | Создает квадратичную кривую Безье
/// [-] canvas.stroke()                 | В действительности рисует определенный вами контур

/// Прямоугольники (4 из 4)
/// [+] canvas.clearRect()              | Очищает заданную область пикселей внутри данного прямоугольника
/// [+] canvas.fillRect()               | Рисует "залитый" прямоугольник
/// [+] canvas.rect()                   | Создает прямоугольник
/// [+] canvas.strokeRect()             | Рисует прямоугольник (без заливки)

/// Вывод изображений (0 из 1)
/// [-] canvas.drawImage()              | Рисует изображение, содержимое другого элемента <canvas> или видео

/// Трансформации (0 из 5)
/// [-] canvas.rotate()                 | Поворачивает текущий графический объект
/// [-] canvas.scale()                  | Изменяет масштаб текущего графического объекта
/// [-] canvas.setTransform()           | Сбрасывает текущую матрицу трансформации в начальное состояние, а затем вызывает метод transform() с теми же параметрами
/// [-] canvas.transform()              | Применяет заданную матрицу трансформации
/// [-] canvas.translate()              | Ретранслирует позицию (0,0) в новое место

/// Другое (0 из 5)
/// [-] canvas.save()                   | Сохраняет состояние текущего контекста
/// [-] canvas.restore()                | Возвращает ранее сохраненное состояние и атрибуты
/// [-] canvas.createEvent()            |
/// [-] canvas.getContext()             |
/// [-] canvas.toDataURL()              |


//uint8_t* jse_createBuffer(){
//    uint8_t *jse_framebuffer_addr;
//    uint32_t jse_framebuffer_size = (getScreenHeight()) * (getDisplayPitch());
//    jse_framebuffer_addr = (uint8_t*)kcalloc(jse_framebuffer_size, 1);
//    qemu_note("[JSE] [Canvas] Create buffer: %x | Size: %d", framebuffer_addr, jse_framebuffer_size);
//    return jse_framebuffer_addr;
//}

jsval_t jse_ext_canvas_fillStyle(struct js *js, jsval_t *args, int nargs) {
    if (nargs != 1) return js_mknum(js->Canvas.fillStyle);
    int c = jse_func_atoi(js_str(js,(args[0])));  // Fetch 1st arg
    qemu_log(" [JSE] [EXT] [Canvas] [fillStyle] Color:%x", c);
    js->Canvas.fillStyle = c;
    return js_mknum(js->Canvas.fillStyle);
}

jsval_t jse_ext_canvas_setPixel(struct js *js, jsval_t *args, int nargs) {
    if (nargs != 3) return js_mkfalse();
    int x = jse_func_atoi(js_str(js,(args[0])));  // Fetch 1st arg
    int y = jse_func_atoi(js_str(js,(args[1])));  // Fetch 2st arg
    int c = jse_func_atoi(js_str(js,(args[2])));  // Fetch 3st arg
    qemu_log(" [JSE] [EXT] [Canvas] [setPixel] X: %d | Y: %d | Color:%x", x, y, c);
    set_pixel(x, y, c);
    return js_mktrue();
}


jsval_t jse_ext_canvas_lineWidth(struct js *js, jsval_t *args, int nargs) {
    if (nargs != 1) return js_mknum(js->Canvas.lineWidth);
    int c = jse_func_atoi(js_str(js,(args[0])));  // Fetch 1st arg
    qemu_log(" [JSE] [EXT] [Canvas] [lineWidth] lineWidth:%d", c);
    js->Canvas.lineWidth = c;
    return js_mknum(js->Canvas.lineWidth);
}


jsval_t jse_ext_canvas_width(struct js *js, jsval_t *args, int nargs) {
    return js_mknum(js->Canvas.width);
}


jsval_t jse_ext_canvas_height(struct js *js, jsval_t *args, int nargs) {
    return js_mknum(js->Canvas.height);
}

jsval_t jse_ext_canvas_fillRect(struct js *js, jsval_t *args, int nargs) {
    if (nargs != 4) return js_mkundef();
    int x = jse_getInt(js,args[0]);  // Fetch 1st arg
    int y = jse_getInt(js,args[1]);  // Fetch 2nd arg
    int w = jse_getInt(js,args[2]);  // Fetch 3nd arg
    int h = jse_getInt(js,args[3]);  // Fetch 4nd arg

    int color = js->Canvas.fillStyle;
    qemu_log(" [JSE] [EXT] [Canvas] [fillRect] X:%d Y:%d W:%d H:%d Color:%x", x, y, w, h, color);
    drawRect(x, y, w, h, color);

    return js_mktrue();
}


jsval_t jse_ext_canvas_clearRect(struct js *js, jsval_t *args, int nargs) {
    if (nargs != 4) return js_mkundef();
    int x = jse_getInt(js,args[0]);  // Fetch 1st arg
    int y = jse_getInt(js,args[1]);  // Fetch 2nd arg
    int w = jse_getInt(js,args[2]);  // Fetch 3nd arg
    int h = jse_getInt(js,args[3]);  // Fetch 4nd arg

    drawRect(x, y, w, h, 0xFF000000);
    qemu_log(" [JSE] [EXT] [Canvas] [clearRect] X:%d Y:%d W:%d H:%d", x, y, w, h);

    return js_mktrue();
}


jsval_t jse_ext_canvas_strokeRect(struct js *js, jsval_t *args, int nargs) {
    if (nargs != 4) return js_mkundef();
    int x = jse_getInt(js,args[0]);          // Fetch 1st arg
    int y = jse_getInt(js,args[1]);          // Fetch 2nd arg
    int w = jse_getInt(js,args[2]);          // Fetch 3nd arg
    int h = jse_getInt(js,args[3]);          // Fetch 4nd arg
    int lineWidth = js->Canvas.lineWidth;   // Ширина линии
    int color = js->Canvas.fillStyle;
    if (lineWidth == 0) return js_mkfalse();

    for (int i = 0; i < lineWidth; i++) {
        int new_width = w + 2 * i;
        int new_height = h + 2 * i;
        set_pixel(x - i, y - i, color);  // Верхняя граница
        set_pixel(x - i + new_width, y - i, color);  // Правая граница
        set_pixel(x - i, y - i + new_height, color);  // Нижняя граница
        set_pixel(x - i + new_width, y - i + new_height, color);  // Левая граница
    }

    for (int i = 0; i < lineWidth; i++) {
        int new_x = x + i;
        int new_y = y + i;
        int new_width = w - 2 * i;
        int new_height = h - 2 * i;
        for (int j = new_y; j < new_y + new_height; j++) {
            set_pixel(new_x, j, color);  // Левая граница
            set_pixel(new_x + new_width, j, color);  // Правая граница
        }
        for (int j = new_x; j < new_x + new_width; j++) {
            set_pixel(j, new_y, color);  // Верхняя граница
            set_pixel(j, new_y + new_height, color);  // Нижняя граница
        }
    }
    qemu_log(" [JSE] [EXT] [Canvas] [strokeRect] X:%d Y:%d W:%d H:%d Color:%x", x, y, w, h, color);

    //drawRect(x, y, w, h, 0xFF000000);

    return js_mktrue();
}


void jse_canvas_config(struct js* js){
    // Установка значений по умолчанию
    JSE_CANVAS cfg =  js->Canvas;
    cfg.fillStyle = 0xFFFFFF;
    cfg.lineWidth = 2;
    cfg.width = 800;
    cfg.height = 600;
    // Регистрация функционала
    qemu_note("[JSE] [EXT] [Canvas] Registration of functions");
    js_set(js, js_glob(js), "canvas_fillRect", js_mkfun(jse_ext_canvas_fillRect));
    js_set(js, js_glob(js), "canvas_clearRect", js_mkfun(jse_ext_canvas_clearRect));
    js_set(js, js_glob(js), "canvas_strokeRect", js_mkfun(jse_ext_canvas_strokeRect));

    js_set(js, js_glob(js), "canvas_height", js_mkfun(jse_ext_canvas_height));
    js_set(js, js_glob(js), "canvas_width", js_mkfun(jse_ext_canvas_width));

    js_set(js, js_glob(js), "canvas_fillStyle", js_mkfun(jse_ext_canvas_fillStyle));
    js_set(js, js_glob(js), "canvas_lineWidth", js_mkfun(jse_ext_canvas_lineWidth));

    /// Допы

    js_set(js, js_glob(js), "canvas_setPixel", js_mkfun(jse_ext_canvas_setPixel));

    /// Алиасы
    js_set(js, js_glob(js), "canvas_rect", js_mkfun(jse_ext_canvas_fillRect));
    return;
}

void jse_canvas_destroy(struct js* js){

}