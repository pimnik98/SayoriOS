#ifndef SAYORI_JSE_ARRAY_H
#define SAYORI_JSE_ARRAY_H

#pragma once
#include "common.h"

#define JSE_EXT_ARRAY_INITIAL_SIZE 16   ///! По-умолчанию создается с объемом

/**
 * @brief Типы данных
 */
typedef enum {
    JSE_ARRAY_TYPE_NULL,                ///! Неизвестный тип данных или поврежден
    JSE_ARRAY_TYPE_INT,                 ///! Число
    JSE_ARRAY_TYPE_STRING,              ///! Строка
    JSE_ARRAY_TYPE_ARRAY                ///! Массив
} JSE_ARRAY_TYPE;

/**
 * @brief Значение ключа
 */
typedef union {
    int32_t int_value;                  ///! Числовое значение (при JSE_ARRAY_TYPE_INT)
    char* str_value;                    ///! Строка (при JSE_ARRAY_TYPE_STRING)
    struct JSE_ARRAY* array_value;      ///! Массив (при JSE_ARRAY_TYPE_ARRAY)
} JSE_ARRAY_VALUE;

/**
 * @brief Данные массива
 */
typedef struct {
    char* key;                          ///! Ключ
    JSE_ARRAY_VALUE value;              ///! Значение
    JSE_ARRAY_TYPE type;                ///! Тип
} JSE_ARRAY_DATA;

/**
 * @brief Структура массива
 */
typedef struct JSE_ARRAY {
    JSE_ARRAY_DATA* pairs;              ///! Данные (ключ,значение)
    int size;                           ///! Кол-во элементов
    int capacity;                       ///! Массив памяти
} JSE_ARRAY;

/// Array
JSE_ARRAY jse_array_create();
JSE_ARRAY* jse_array_link_create();
void jse_array_link_dump(JSE_ARRAY* arr, int space);
void jse_array_dump(JSE_ARRAY arr,int space);
void jse_array_value_dump(JSE_ARRAY_DATA data, int space);
void jse_array_push(JSE_ARRAY* arr, const char* key, JSE_ARRAY_VALUE value, JSE_ARRAY_TYPE type);
JSE_ARRAY_DATA jse_array_getByKey(JSE_ARRAY* arr, const char* key);
JSE_ARRAY_DATA jse_array_getByIndex(JSE_ARRAY* arr, int index);
void jse_array_free(JSE_ARRAY* arr);
JSE_ARRAY jse_array_change_key_case(const JSE_ARRAY* orig, int uppercase);
void jse_array_fill(JSE_ARRAY* arr, int start_index, int count, JSE_ARRAY_VALUE value, JSE_ARRAY_TYPE type);
JSE_ARRAY jse_array_diff(JSE_ARRAY* array, JSE_ARRAY* other_array);
void jse_array_destroZ(JSE_ARRAY* array, int space);
void jse_array_editByID(JSE_ARRAY* arr, int Index, JSE_ARRAY_VALUE value, JSE_ARRAY_TYPE type);

#endif //SAYORI_JSE_ARRAY_H
