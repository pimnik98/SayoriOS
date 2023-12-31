#ifndef SAYORI_JSE_CANVAS_H
#define SAYORI_JSE_CANVAS_H
#pragma once

#include "common.h"
/// Структура экрана/холста Canvas - все еще может быть изменина пока не будет доведено, до финальной версии
typedef struct {
    int Init;				        ///< Инициализировано?
    int x;				            ///< Позиция по Х
    int y;				            ///< Позиция по Y
    uint8_t* data;                  ///< Ссылка на буфер
    int height;                     ///< Высота экрана
    int width;                      ///< Ширина экрана
    uint32_t fillStyle;                  ///< Цвет заливки
    int shadowBlur;                 ///< Уровень размытости для теней
    int shadowColor;                ///< Цвет для теней
    int shadowOffsetX;              ///< Горизонтальное расстояние тени от фигуры
    int shadowOffsetY;              ///< Вертикальное расстояние тени от фигуры
    int strokeStyle;                ///< Цвет, градиент или шаблон, используемый для обводки фигуры
    int font;                       ///< Cвойства шрифта для текстового содержимого
    int textAlign;                  ///< Выравнивание для текстового содержимого
    int textBaseline;               ///< Базовая линия, используемая при выводе текста
    int globalAlpha;                ///< Текущее значение прозрачности или альфа-канала
    int globalCompositeOperation;   ///< Как исходное (новое) изображение нарисовано на экране
    int lineCap;                    ///< Стиль концов нарисованной линии
    int lineJoin;                   ///< Тип угла, созданного пересечением двух линий
    int lineWidth;                  ///< Ширина текущей линии
} __attribute__((packed)) JSE_CANVAS;

#endif //SAYORI_JSE_CANVAS_H
