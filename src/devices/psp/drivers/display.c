#include <pspge.h>
#include <pspdisplay.h>
#include <psputils.h>
#include "../psp.h"
#include "display.h"
#include "../../loader.h"

uint32_t* psp_display_buf1;
uint32_t* psp_display_buf2;

/**
 * @brief Комбинирует 8-битные значения альфа, красного, зеленого и синего каналов в 32-битное значение цвета.
 *
 * Эта функция берет четыре 8-битных значения, представляющих альфа, красный, зеленый и синий каналы цвета, 
 * и объединяет их в 32-битное значение цвета. Результирующее значение цвета имеет формат ARGB, где альфа-канал
 * занимает старшие 8 бит, красный канал следующие 8 бит, зеленый канал следующие 8 бит и синий канал занимает
 * младшие 8 бит.
 *
 * @param a 8-битное значение альфа-канала (0-255).
 * @param r 8-битное значение красного канала (0-255).
 * @param g 8-битное значение зеленого канала (0-255).
 * @param b 8-битное значение синего канала (0-255).
 *
 * @return 32-битное значение цвета в формате ARGB.
 *
 * @code
 *  uint32_t color = psp_display_combine_color_channels(255, 100, 50, 0);
 *  // color будет равно 0xFF643200
 * @endcode
 * @date 03.01.2025
 * @author SayoriOS Team | pimnik98
 * @version 0.4.0
 */
uint32_t psp_display_combine_color_channels(uint8_t a, uint8_t r, uint8_t g, uint8_t b) {
    uint32_t color = 0;
    color |= ((uint32_t)a << 24);  // Помещаем значение альфа канала в старшие биты
    color |= ((uint32_t)r << 16);  // Помещаем значение красного канала в старшие биты
    color |= ((uint32_t)g << 8);   // Помещаем значение зеленого канала в средние биты
    color |= b;                    // Помещаем значение синего канала в младшие биты
    return color;
}

/**
 * @brief Инициализирует дисплей PSP.
 *
 * Эта функция настраивает дисплей PSP, устанавливая режим отображения, размер экрана
 * и буферы кадров. Она получает адреса буферов кадров из EDRAM (встроенной памяти PSP)
 * и устанавливает режим дисплея. 
 * 
 * @details
 *  Функция выделяет два буфера для отображения кадров из EDRAM и настраивает режим дисплея.
 *  Размер экрана задаётся шириной 480 пикселей и высотой 272 пикселя.
 *  Буферы кадров выделяются после области памяти, используемой для графического движка (GE).
 *  Использование функции `sceGeEdramGetAddr()` указывает на работу с графическим движком PSP.
 *  Функция `sceDisplaySetMode()` устанавливает режим отображения и частоту обновления экрана.
 *
 * @note
 *   Буферы кадров (`psp_display_buf1` и `psp_display_buf2`) имеют размер 272 x 512 пикселей.
 *   Буферы выделяются с некоторого смещения в EDRAM для размещения после буфера GE.
 *   Важно обеспечить корректное выделение памяти, чтобы избежать проблем с отображением.
 *   Использование `sceDisplaySetMode(0, 480, 272)` задает режим отображения с шириной 480 пикселей, 
 *   высотой 272 пикселя, и использует автоматический выбор частоты кадров.
 *
 * @code
 *  psp_display_init(); 
 *  // После вызова дисплей PSP будет инициализирован.
 * @endcode
 *
 * @sa sceGeEdramGetAddr, sceDisplaySetMode
 * @date 03.01.2025
 * @author SayoriOS Team | pimnik98
 * @version 0.4.0
 */
void psp_display_init(){
    psp_display_buf1 = (uint32_t*) sceGeEdramGetAddr() + (272 * 512 * 4);
    psp_display_buf2 = (uint32_t*) sceGeEdramGetAddr() + (272 * 512 * 4);
    sceDisplaySetMode(0, 480, 272);
}

/**
 * @brief Возвращает ширину экрана PSP в пикселях.
 *
 * Эта функция возвращает ширину экрана PSP, заданную в константе `SCREEN_W`.
 *
 * @return Ширина экрана в пикселях.
 *
 * @code
 *  uint32_t width = psp_display_width(); // width будет содержать ширину экрана
 * @endcode
 * @date 03.01.2025
 * @author SayoriOS Team | pimnik98
 * @version 0.4.0
 */
uint32_t psp_display_width(){
    return SCREEN_W;
}

/**
 * @brief Возвращает высоту экрана PSP в пикселях.
 *
 * Эта функция возвращает высоту экрана PSP, заданную в константе `SCREEN_H`.
 *
 * @return Высота экрана в пикселях.
 *
 * @code
 *  uint32_t height = psp_display_height(); // height будет содержать высоту экрана
 * @endcode
 * @date 03.01.2025
 * @author SayoriOS Team | pimnik98
 * @version 0.4.0
 */
uint32_t psp_display_height(){
    return SCREEN_H;
}

/**
 * @brief Очищает буфер экрана заданным цветом.
 *
 * Эта функция заполняет весь буфер кадра (`psp_display_buf1`) заданным цветом. 
 * Это эквивалентно очистке экрана и заполнению его однородным цветом.
 *
 * @param color 32-битное значение цвета, которым будет заполнен экран.
 *
 * @details
 * Функция итерируется по всем пикселям буфера кадра и устанавливает 
 * значение каждого пикселя в заданный цвет.
 * Предполагается, что размер буфера кадра SCREEN_W * SCREEN_H пикселей.
 *
 * @note
 *  Данная функция очищает только `psp_display_buf1`. Для двойной буферизации может потребоваться
 *  очистка и второго буфера.
 *
 * @code
 *  uint32_t black = 0xFF000000; // Черный цвет
 *  psp_display_clear(black);   // Очищает экран черным цветом
 * @endcode
 *
 * @see psp_display_buf1
 * @date 03.01.2025
 * @author SayoriOS Team | pimnik98
 * @version 0.4.0
 */
void psp_display_clear(uint32_t color){
    for (int i = 0; i < SCREEN_W * SCREEN_H; i++){
        psp_display_buf1[i] = color;
    }
}

/**
 * @brief Обновляет дисплей PSP, переключая буферы и отображая новый кадр.
 *
 * Эта функция выполняет двойную буферизацию дисплея PSP. Она переключает буферы
 * кадров и устанавливает новый буфер для отображения на следующем кадре,
 * также синхронизирует кеш данных с основной памятью.
 *
 * @details
 *  Функция использует временную переменную (`temp`) для переключения указателей
 *  на буферы кадров (`psp_display_buf1` и `psp_display_buf2`). После переключения
 *  буферов, функция вызывает `sceKernelDcacheWritebackInvalidateAll()` для
 *  синхронизации кеша данных, чтобы гарантировать, что изменения в буфере
 *  кадров будут отображены на экране. Затем она вызывает `sceDisplaySetFrameBuf()`
 *  для установки нового буфера кадров для отображения на следующем кадре.
 *
 * @note
 *  Важно, чтобы перед вызовом этой функции буфер кадров (`psp_display_buf1`) был
 *  полностью отрисован. Переключение буферов кадра должно происходить синхронно
 *  с вертикальной синхронизацией, чтобы избежать артефактов.
 *  `PSP_DISPLAY_PIXEL_FORMAT_8888` задает формат пикселей, который использует 32 бита
 *  на пиксель (8 бит на красный, зеленый, синий и альфа каналы).
 *  `PSP_DISPLAY_SETBUF_NEXTFRAME` указывает, что буфер кадра будет отображен
 *  на следующем кадре.
 *
 * @code
 *  // ... код отрисовки в psp_display_buf1 ...
 *  psp_display_update(); // Обновляет дисплей
 *  // ... код отрисовки в psp_display_buf1 для следующего кадра ...
 * @endcode
 *
 * @see psp_display_buf1, psp_display_buf2, sceKernelDcacheWritebackInvalidateAll, sceDisplaySetFrameBuf
 * @date 03.01.2025
 * @author SayoriOS Team | pimnik98
 * @version 0.4.0
 */
void psp_display_update(){
    uint32_t* temp = psp_display_buf2;
    psp_display_buf1 = psp_display_buf2;
    psp_display_buf1 = temp;
    sceKernelDcacheWritebackInvalidateAll();
    sceDisplaySetFrameBuf(psp_display_buf2, 512, PSP_DISPLAY_PIXEL_FORMAT_8888, PSP_DISPLAY_SETBUF_NEXTFRAME);
}

/**
 * @brief Устанавливает цвет пикселя в буфере кадра по заданным координатам.
 *
 * Эта функция устанавливает цвет пикселя с координатами (x, y) в буфере кадра (`psp_display_buf1`).
 * Она выполняет проверку границ, чтобы убедиться, что координаты находятся в пределах экрана.
 *
 * @param x Координата X пикселя (горизонтальная).
 * @param y Координата Y пикселя (вертикальная).
 * @param color 32-битное значение цвета, который нужно установить для пикселя.
 *
 * @details
 *  Функция вычисляет смещение пикселя в буфере кадра, используя формулу `x + (y * 512)`, где 512
 *  — ширина буфера кадров. Она проверяет, что координаты `x` и `y` находятся в пределах экрана
 *  (определено константами `SCREEN_W` и `SCREEN_H`). Если координаты выходят за границы, функция
 *  не выполняет никаких действий.
 *  Цвет, заданный параметром `color`, переупорядочивается в RGBA формат с помощью функции `psp_display_combine_color_channels`
 *  и сохраняется в буфере кадра.
 *
 * @note
 *  Функция изменяет только `psp_display_buf1`. После вызова этой функции необходимо
 *  вызвать `psp_display_update()` для отображения изменений на экране.
 *
 * @code
 *  uint32_t red = 0xFFFF0000; // Красный цвет
 *  psp_display_set_pixel(10, 20, red); // Устанавливает красный пиксель в координатах (10, 20)
 * @endcode
 *
 * @see psp_display_buf1, psp_display_update, psp_display_combine_color_channels
 * @date 03.01.2025
 * @author SayoriOS Team | pimnik98
 * @version 0.4.0
 */
void psp_display_set_pixel(unsigned int x, unsigned int y, uint32_t color){
    if (x < 0 || x > SCREEN_W) return;
    if (y < 0 || y > SCREEN_H) return;
    
    int off = x + (y * 512);

    int a = (color >> 24);
	int r = (color >> 16);
	int g = (color >> 8);
	int b = color;

    psp_display_buf1[off] = psp_display_combine_color_channels(a,b,g,r);
}

/**
 * @brief Рисует прямоугольник заданным цветом на экране.
 *
 * Эта функция рисует прямоугольник заданного размера и цвета в указанной позиции на экране.
 * Она использует функцию `psp_display_set_pixel` для установки цвета каждого пикселя прямоугольника.
 *
 * @param x Координата X верхнего левого угла прямоугольника.
 * @param y Координата Y верхнего левого угла прямоугольника.
 * @param w Ширина прямоугольника в пикселях.
 * @param h Высота прямоугольника в пикселях.
 * @param color 32-битное значение цвета, которым нужно заполнить прямоугольник.
 *
 * @details
 *  Функция итерируется по всем пикселям прямоугольника, определяемого его шириной `w`, высотой `h`,
 *  и координатами верхнего левого угла `(x, y)`. Для каждого пикселя она вызывает функцию
 *  `psp_display_set_pixel`, чтобы установить соответствующий цвет.
 *
 * @note
 *  Эта функция рисует прямоугольник, используя `psp_display_set_pixel`, поэтому
 *  она будет относительно медленной для больших прямоугольников.
 *  Для отображения изменений на экране необходимо вызвать `psp_display_update()`.
 *
 * @code
 *  uint32_t blue = 0xFF0000FF; // Синий цвет
 *  psp_display_draw_rect(50, 50, 100, 50, blue); // Рисует синий прямоугольник в координатах (50, 50) размером 100x50
 * @endcode
 *
 * @see psp_display_set_pixel, psp_display_update
 * @date 03.01.2025
 * @author SayoriOS Team | pimnik98
 * @version 0.4.0
 */
void psp_display_draw_rect(unsigned int x, unsigned int y, unsigned int w, unsigned int h, uint32_t color){
    for (int y1 = 0; y1 < h; y1++){
        for (int x1 = 0; x1 <w;x1++){
            psp_display_set_pixel(x+x1, y+y1, color);
        }
    }
}