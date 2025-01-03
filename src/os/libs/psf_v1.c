/**
 * @file src/os/libs/psf_v1.c
 * @author Пиминов Никита (nikita.piminoff@yandex.ru)
 * @brief Поддержка шрифтов PSF v1
 * @version 0.4.0
 * @date 2025-01-03
 * @copyright Copyright SayoriOS Team (c) 2025
 */
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "psf_v1.h"
#include "../../devices/loader.h"

uint32_t psf_font_version           = 0;
static int _w                       = 8;            /**< Ширина символа в пикселях (всегда 8). */
static int _h                       = 0;            /**< Высота символа в пикселях. */
static psf_t* _font_ptr             = nullptr;      /**< Указатель на начало данных PSF шрифта. */
static uint8_t* first_glyph         = 0;            /**< Указатель на начало данных глифов. */
static bool _init                   = false;
uint16_t *unicode;

/**
 * @brief Инициализирует шрифт PSF из файла.
 *
 * @param psf Путь к файлу шрифта PSF.
 * @return 1 в случае успеха, 2 если `psf` равен `NULL`, 3 если не удалось открыть файл,
 *         4 если магическое число PSF неверно.
 *
 * @details  Функция открывает файл шрифта PSF, считывает его содержимое в буфер,
 *           проверяет магическое число и инициализирует глобальные переменные, 
 *           необходимые для отрисовки шрифта.
 * 
 * @date        03.01.2025
 * @author      SayoriOS Team | pimnik98
 * @version     0.4.0
 */
int psf1_init(char* psf){
	ON_NULLPTR(psf, {
		return 2;
	});

    FILE* psf_file = fopen(psf, "r");
    if (!psf_file) {
        return 3;
    }

    fseek(psf_file, 0, SEEK_END);
    size_t rfsize = ftell(psf_file);
    fseek(psf_file, 0, SEEK_SET);

    char* buffer = malloc(rfsize);
	fread(buffer, sizeof(char), rfsize, psf_file);
    fclose(psf_file);

    psf_t *header = (psf_t*)buffer;
    _w = 0;
    _h = 0;
    if (header->magic[0] != PSF1_MAGIC0 || header->magic[1] != PSF1_MAGIC1){
        return 4;
    }
    _font_ptr = (psf_t*)buffer;
    _w = 8;
    _h = header->charHeight;
    first_glyph = (uint8_t*)_font_ptr+sizeof(psf_t);
    return 1;
}

/**
 * @brief Возвращает ширину символа шрифта PSF.
 *
 * @return Ширина символа в пикселях.
 * @date        03.01.2025
 * @author      SayoriOS Team | pimnik98
 * @version     0.4.0
 */
size_t psf1_get_w() {
    return _w;
}

/**
 * @brief Возвращает высоту символа шрифта PSF.
 *
 * @return Высота символа в пикселях.
 * @date        03.01.2025
 * @author      SayoriOS Team | pimnik98
 * @version     0.4.0
 */
size_t psf1_get_h() {
    return _h;
}

/**
 * @brief Применяет патч к символам кириллицы в шрифте PSF.
 *
 * @param c     Первый байт UTF-16 символа.
 * @param c2    Второй байт UTF-16 символа.
 * @return      Замененный код символа, либо исходный код `c` если нет соответствия для замены, либо 1, если `x` вне диапазона.
 *
 * @details Функция выполняет замену кодов символов кириллицы на основе патча, 
 *          используя  первый и второй байты UTF-16 символа.
 *          Если переданный код не является кириллическим UTF-16 кодом, или код не требует замены, возвращается исходный код `c`.
 *          Если `c2 & 0x3F` не попадает в допустимый диапазон, то возвращается 1.
 * 
 * @date        03.01.2025
 * @author      SayoriOS Team | pimnik98
 * @version     0.4.0
 */
uint16_t psf1_rupatch(uint16_t c, uint16_t c2){
    if (!isUTF(c)) return c;
    if ((c & 0x1F) != 16 && (c & 0x1F) != 17) return c;
    uint16_t x = (c2 & 0x3F);
    uint16_t lS = 224;
    uint16_t bS = 128;
     switch(x){
        case  1: return ((c & 0x1F) == 16?lS+16:lS+1);
        case 17: return ((c & 0x1F) == 17?lS+17:bS+1);
        case  0: return lS+0;     case 22: return bS+6;     case 42: return bS+26;
        case  2: return lS+2;     case 23: return bS+7;     case 43: return bS+27;
        case  3: return lS+3;     case 24: return bS+8;     case 44: return bS+28;
        case  4: return lS+4;     case 25: return bS+9;     case 45: return bS+29;
        case  5: return lS+5;     case 26: return bS+10;    case 46: return bS+30;
        case  6: return lS+6;     case 27: return bS+11;    case 47: return bS+31;
        case  7: return lS+7;     case 28: return bS+12;    case 48: return bS+32;
        case  8: return lS+8;     case 29: return bS+13;    case 49: return bS+33;
        case  9: return lS+9;     case 30: return bS+14;    case 50: return bS+34;
        case 10: return lS+10;    case 31: return bS+15;    case 51: return bS+35;
        case 11: return lS+11;    case 32: return bS+16;    case 52: return bS+36;
        case 12: return lS+12;    case 33: return bS+17;    case 53: return bS+37;
        case 13: return lS+13;    case 34: return bS+18;    case 54: return bS+38;
        case 14: return lS+14;    case 35: return bS+19;    case 55: return bS+39;
        case 15: return lS+15;    case 36: return bS+20;    case 56: return bS+40;
        case 16: return bS+0;     case 37: return bS+21;    case 57: return bS+41;
        case 18: return bS+2;     case 38: return bS+22;    case 58: return bS+42;
        case 19: return bS+3;     case 39: return bS+23;    case 59: return bS+43;
        case 20: return bS+4;     case 40: return bS+24;    case 60: return bS+44;
        case 21: return bS+5;     case 41: return bS+25;    case 61: return bS+45;
        case 62: return bS+46;    case 63: return bS+47;
        default: return 1;
    }
}

/**
 * @brief Возвращает указатель на данные глифа из шрифта PSF.
 *
 * @param ch    Код символа, для которого требуется получить глиф.
 * @return      Указатель на начало данных глифа или 0, если символ находится вне диапазона.
 *
 * @details     Функция вычисляет адрес данных глифа в памяти шрифта PSF на основе кода символа и высоты символа.
 *              Проверяет, находится ли символ в допустимом диапазоне кодов, учитывая режим шрифта.
 *              Если код символа выходит за допустимый диапазон, возвращает 0.
 * 
 * 
 * @date        03.01.2025
 * @author      SayoriOS Team | pimnik98
 * @version     0.4.0
 */
uint8_t *psf1_get_glyph(uint16_t ch){
    psf_t *header = (psf_t*)_font_ptr;

    if ((ch > 511) || (ch > 255 && (header->mode == 0 || header->mode == 2))){
        return 0;
    }
    return ((uint8_t*)_font_ptr+sizeof(psf_t)+(ch*_h));
}

/**
 * @brief Отрисовывает символ из шрифта PSF на заданных координатах.
 *
 * @param c     Первый байт UTF-16 символа.
 * @param c2    Второй байт UTF-16 символа.
 * @param pos_x Координата X верхнего левого угла символа.
 * @param pos_y Координата Y верхнего левого угла символа.
 * @param color Цвет символа.
 *
 * @details     Функция получает глиф символа из шрифта PSF, применяет патч для кириллицы
 *              и отрисовывает глиф попиксельно на заданных координатах с заданным цветом, используя функцию `display_set_pixel`.
 *              Если глиф не найден, то функция ничего не отрисовывает.
 * 
 * @date        03.01.2025
 * @author      SayoriOS Team | pimnik98
 * @version     0.4.0
 */
void psf1_write_ch(uint16_t c, uint16_t c2, size_t pos_x, size_t pos_y, size_t color) {
    char mask[8] = {128,64,32,16,8,4,2,1};
    uint8_t *glyph = psf1_get_glyph(psf1_rupatch(c, c2));

	if(!glyph) return;

    for (size_t y = 0; y < _h; y++){
        for (size_t x = 0; x < _w; x++){
            if (glyph[y] & mask[x]) {
                display_set_pixel(pos_x+x, pos_y+y, color);
            }
        }
    }
}

/**
 * @brief Отрисовывает строку текста с использованием шрифта PSF.
 *
 * @param text  Указатель на начало строки текста.
 * @param len   Длина строки текста.
 * @param x     Координата X верхнего левого угла первого символа.
 * @param y     Координата Y верхнего левого угла всех символов.
 * @param color Цвет текста.
 *
 * @details     Функция проходит по символам в строке, проверяет, являются ли символы UTF-16,
 *              и вызывает функцию `draw_vga_ch` для отрисовки каждого символа.
 *              Отрисовка происходит слева направо.
 *              Если символ UTF-16, отрисовываются два байта, иначе один.
 *              Если координаты символа выходят за пределы экрана по горизонтали, отрисовка прекращается.
 *              Функция возвращает управление, если строка равна NULL.
 * 
 * @date        03.01.2025
 * @author      SayoriOS Team | pimnik98
 * @version     0.4.0
 */
void psf1_write_str(const char* text, size_t len, int x, int y, uint32_t color){
	ON_NULLPTR(text, {
		return;
	});

    size_t scrwidth = display_width();
    for(int i = 0; i < len; i++){
        if (x + _w <= scrwidth){
            if (isUTF(text[i])){
                psf1_write_ch(text[i], text[i+1], x, y, color);
                i++;
            } else {
                if(!text[i])
                    return;
                psf1_write_ch(text[i], 0, x, y, color);
            }
            x += _w;
        } else {
            break;
        }
    }
}