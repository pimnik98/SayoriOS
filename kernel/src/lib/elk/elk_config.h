#pragma once

#include "common.h"

#define     JSE_LIBS_PATH           "R:\\Sayori\\JSE\\"     ///! Путь с библиотеками
#define     JSE_MIN_STACK           8196                    ///! Минимальный размер стэка

#define     JSE_MSG_ERROR_LIBRARY   "Library error!"        ///! Сообщение при ошибки, если проблемы в библиотеках

#define     JSE_DISPLAY_SETPIXEL(x,y,c); set_pixel(x,y,c);  ///! Отрисовка одного пикселя