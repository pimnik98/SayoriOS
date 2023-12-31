/// JavaScript Engine - Поддержка массивов (как в PHP)
#include "portability.h"
#include <io/ports.h>
#include "io/tty.h"
#include "lib/stdio.h"

#include "../elk_config.h"
#include "../elk.h"

JSE_ARRAY JSE_GLOBAL_ARRAY;

/**
 * @brief - [JSE] [EXT] [Array] Создание массива
 *
 * @return - JSE_ARRAY - массив
 */
JSE_ARRAY jse_array_create() {
    JSE_ARRAY* arr = (JSE_ARRAY*)calloc(1, sizeof(JSE_ARRAY));
    arr->pairs = (JSE_ARRAY_DATA*)calloc(1, JSE_EXT_ARRAY_INITIAL_SIZE * sizeof(JSE_ARRAY_DATA));
    arr->size = 0;
    arr->capacity = JSE_EXT_ARRAY_INITIAL_SIZE;
    return arr[0];
}


/**
 * @brief - [JSE] [EXT] [Array] Создание массива
 *
 * @return - JSE_ARRAY* - массив
 */
JSE_ARRAY* jse_array_link_create() {
    JSE_ARRAY* arr = (JSE_ARRAY*) calloc(1, sizeof(JSE_ARRAY));
    arr->pairs = (JSE_ARRAY_DATA*) calloc(1, JSE_EXT_ARRAY_INITIAL_SIZE * sizeof(JSE_ARRAY_DATA));
    arr->size = 0;
    arr->capacity = JSE_EXT_ARRAY_INITIAL_SIZE;
    return arr;
}
/**
 * @brief - [JSE] [EXT] [Array] Отладочная информация для массивов
 *
 * @param JSE_ARRAY arr - Массив
 * @param int space - Кол-во пробелов
 */
void jse_array_dump(JSE_ARRAY arr, int space){
    char* sp = (char*) calloc(1, (sizeof(char) * (space + 1)));
    memset(sp,' ', space);
    for (int i = 0; i < arr.size; i++) {
        if (arr.pairs[i].type == JSE_ARRAY_TYPE_INT) {
            printf("%s[%d] Key: %s, Value (INT): %d\n",sp, i, arr.pairs[i].key, arr.pairs[i].value.int_value);
        } else if (arr.pairs[i].type == JSE_ARRAY_TYPE_STRING) {
            printf("%s[%d] Key: %s, Value (STRING): %s\n",sp, i, arr.pairs[i].key, arr.pairs[i].value.str_value);
        } else if (arr.pairs[i].type == JSE_ARRAY_TYPE_ARRAY) {
            printf("%s[%d] Key: %s, Value (ARRAY): Size %d\n",sp, i, arr.pairs[i].key, arr.pairs[i].value.array_value->size);
            jse_array_dump(arr.pairs[i].value.array_value[0],space + 4);
        }
    }
    free(sp);
}

/**
 * @brief - [JSE] [EXT] [Array] Отладочная информация для массивов
 *
 * @param JSE_ARRAY arr - Массив
 * @param int space - Кол-во пробелов
 */
void jse_array_link_dump(JSE_ARRAY* arr, int space){
    char* sp = (char*)calloc(1, sizeof(char) * (space + 1));
    memset(sp,' ', space);
    for (int i = 0; i < arr->size; i++) {
        if (arr->pairs[i].type == JSE_ARRAY_TYPE_INT) {
            printf("%s[%d] Key: %s, Value (INT): %d\n",sp, i, arr->pairs[i].key, arr->pairs[i].value.int_value);
        } else if (arr->pairs[i].type == JSE_ARRAY_TYPE_STRING) {
            printf("%s[%d] Key: %s, Value (STRING): %s\n",sp, i, arr->pairs[i].key, arr->pairs[i].value.str_value);
        } else if (arr->pairs[i].type == JSE_ARRAY_TYPE_ARRAY) {
            printf("%s[%d] Key: %s, Value (ARRAY): Size %d\n",sp, i, arr->pairs[i].key, arr->pairs[i].value.array_value->size);
            jse_array_dump(arr->pairs[i].value.array_value[0],space + 4);
        }
    }
    free(sp);
}

/**
 * @brief - [JSE] [EXT] [Array] Отладочная информация для значений
 *
 * @param JSE_ARRAY_DATA data - Данные массива (ключ и его значения)
 * @param int space - Кол-во пробелов
 */
void jse_array_value_dump(JSE_ARRAY_DATA data, int space){
    if (data.type == JSE_ARRAY_TYPE_NULL) {
        printf("[JSE] [Array] [DUMP] [VALUE] Invalid data\n");
        return;
    }
    char* sp = (char*) calloc(1, sizeof(char) * (space + 1));
    memset(sp,' ', space);
    printf("[JSE] [Array] [DUMP] [VALUE] Success!\n%s Key: %s\n", sp, data.key);
    printf("%s  |--- Type: %d\n",sp, data.type);
    if (data.type == JSE_ARRAY_TYPE_INT){
        printf("%s  |--- Value: %d\n",sp, data.value.int_value);
    } else if (data.type == JSE_ARRAY_TYPE_STRING){
        printf("%s  |--- Value: %s\n",sp, data.value.str_value);
    } else if (data.type == JSE_ARRAY_TYPE_ARRAY){
        printf("%s  |--- Value: (Array)\n",sp);
        jse_array_dump(data.value.array_value[0],space + 4);

    }
    free(sp);
}

/**
 * @brief - [JSE] [EXT] [Array] Добавление в конец массива элемента
 *
 * @param JSE_ARRAY* arr - Ссылка на массив
 * @param const char* key - Имя ключа
 * @param JSE_ARRAY_VALUE value - Данные для чтения|записи в массив
 * @param JSE_ARRAY_TYPE type - Тип данных (смотрите в JSE_ARRAY_TYPE)
 */
void jse_array_push(JSE_ARRAY* arr, const char* key, JSE_ARRAY_VALUE value, JSE_ARRAY_TYPE type) {
    if (arr->size >= arr->capacity) {
        arr->capacity *= 2;
        arr->pairs = (JSE_ARRAY_DATA*)realloc(arr->pairs, arr->capacity * sizeof(JSE_ARRAY_DATA));
    }
    arr->pairs[arr->size].key = jse_strdup(key);
    arr->pairs[arr->size].value = value;
    arr->pairs[arr->size].type = type;
    arr->size++;
}


/**
 * @brief - [JSE] [EXT] [Array] Добавление в конец массива элемента
 *
 * @param JSE_ARRAY* arr - Ссылка на массив
 * @param const char* key - Имя ключа
 * @param JSE_ARRAY_VALUE value - Данные для чтения|записи в массив
 * @param JSE_ARRAY_TYPE type - Тип данных (смотрите в JSE_ARRAY_TYPE)
 */
void jse_array_editByID(JSE_ARRAY* arr, int Index, JSE_ARRAY_VALUE value, JSE_ARRAY_TYPE type) {
    if (Index >= 0 && Index < arr->size) {
        JSE_ARRAY_VALUE val = arr->pairs[Index].value;
        if (arr->pairs[Index].type == JSE_ARRAY_TYPE_ARRAY){
            jse_array_destroZ(val.array_value, 0);
        } else if (arr->pairs[Index].type == JSE_ARRAY_TYPE_STRING){
            free(val.str_value);
        }
        arr->pairs[Index].value = value;
        arr->pairs[Index].type = type;
    } else {
        qemu_err("Error: Index out of bounds\n");
    }
}

/**
 * @brief - [JSE] [EXT] [Array] Получение данных по ключу
 *
 * @param JSE_ARRAY* arr - Ссылка на массив
 * @param const char* key - Ключ
 *
 * @return JSE_ARRAY_DATA - Ключ, тип и значение
 */
JSE_ARRAY_DATA jse_array_getByKey(JSE_ARRAY* arr, const char* key) {
    for (int i = 0; i < arr->size; i++) {
        if (strcmp(arr->pairs[i].key, key) == 0) {
            //jse_array_value_dump(arr->pairs[i], 4);
            return arr->pairs[i];
            //return arr->pairs[i].value;
        }
    }
    JSE_ARRAY_DATA jse_arr = {};
    return jse_arr;  // Возврат, если ключ не найден
}

/**
 * @brief - [JSE] [EXT] [Array] Получение данных по индексу
 *
 * @param JSE_ARRAY* arr - Ссылка на массив
 * @param int index - Индекс
 *
 * @return JSE_ARRAY_DATA - Ключ, тип и значение
 */
JSE_ARRAY_DATA jse_array_getByIndex(JSE_ARRAY* arr, int index) {
    if (index >= 0 && index < arr->size) {
        return arr->pairs[index];
    }

    JSE_ARRAY_DATA jse_arr = {};
    return jse_arr;
}

/**
 * @brief - [JSE] [EXT] [Array] Освобождение памяти массива из ОЗУ
 *
 * @param JSE_ARRAY* arr - Ссылка на массив
 */
void jse_array_free(JSE_ARRAY* arr) {
    for (int i = 0; i < arr->size; i++) {
        //qemu_note("[Array] 1.Free %x",arr->pairs[i].key);
        free(arr->pairs[i].key);
        if (arr->pairs[i].type == JSE_ARRAY_TYPE_NULL ||
            arr->pairs[i].type != JSE_ARRAY_TYPE_STRING ||
            arr->pairs[i].type != JSE_ARRAY_TYPE_INT ||
            arr->pairs[i].type != JSE_ARRAY_TYPE_ARRAY
                ){
            qemu_log("[Array] Skipping data");
            continue;
        }
        if (arr->pairs[i].type == JSE_ARRAY_TYPE_STRING) {
            free(arr->pairs[i].value.str_value);
        } else if (arr->pairs[i].type == JSE_ARRAY_TYPE_ARRAY) {
            // Дополнительная логика для освобождения памяти массива
            qemu_note("[Array] In %x",arr->pairs[i].value.array_value);
            jse_array_free(arr->pairs[i].value.array_value);
        }
    }
    qemu_note("[Array] 2.Free %x",arr->pairs);
    free(arr->pairs);
    arr->size = 0;
    arr->capacity = 0;
    qemu_note("[Array] 3.Free %x",arr);
    free(arr);
}
/**
 * @brief - [JSE] [EXT] [Array] Меняет регистр всех ключей в массиве
 *
 * @param const JSE_ARRAY* orig - Ссылка на массив
 * @param int uppercase - Регистр (1 - большие | 0 - маленькие)
 *
 * @return - Возвращает массив, где все ключи которого преобразованы в нижний или верхний регистр.
 */

JSE_ARRAY jse_array_change_key_case(const JSE_ARRAY* orig, int uppercase) {
    JSE_ARRAY new_arr = jse_array_create();

    for (int i = 0; i < orig->size; i++) {
        char* new_key = jse_strdup(orig->pairs[i].key);
        (uppercase ?jse_func_toupper(new_key):jse_func_tolower(new_key));
        jse_array_push(&new_arr, new_key, orig->pairs[i].value, orig->pairs[i].type);
    }

    return new_arr;
}
/**
 * @brief - [JSE] [EXT] [Array] Заполнение однотипными значениями в массив
 *
 * @param JSE_ARRAY* arr - Ссылка на массив
 * @param int start_index - Откуда начинаем
 * @param int count - Количество
 * @param JSE_ARRAY_VALUE value - Данные для записи в массив
 * @param JSE_ARRAY_TYPE type - Тип данных (смотрите в JSE_ARRAY_TYPE)
 */

void jse_array_fill(JSE_ARRAY* arr, int start_index, int count, JSE_ARRAY_VALUE value, JSE_ARRAY_TYPE type) {
    for (int i = start_index; i < start_index + count; i++) {
        if (i < arr->size) {
            if (arr->pairs[i].type == JSE_ARRAY_TYPE_STRING) {
                free(arr->pairs[i].value.str_value);
            } else if (arr->pairs[i].type == JSE_ARRAY_TYPE_ARRAY) {
                jse_array_free(arr->pairs[i].value.array_value);
            }
            arr->pairs[i].value = value;
            arr->pairs[i].type = type;
        } else {
            if (arr->size >= arr->capacity) {
                arr->capacity *= 2;
                arr->pairs = (JSE_ARRAY_DATA*)realloc(arr->pairs, arr->capacity * sizeof(JSE_ARRAY_DATA));
            }
            arr->pairs[i].key = jse_strdup("");
            arr->pairs[i].value = value;
            arr->pairs[i].type = type;
            arr->size++;
        }
    }
}
/**
 * @brief - [JSE] [EXT] [Array] Сравнить два массива и создать новый массив с расхождениями
 *
 * @param JSE_ARRAY* array - Ссылка на массив 1
 * @param JSE_ARRAY* other_array - Ссылка на массив 2
 *
 * @return - Массив с расхождениями
 */

JSE_ARRAY jse_array_diff(JSE_ARRAY* array, JSE_ARRAY* other_array) {
    JSE_ARRAY diff_array = jse_array_create();  // Создаем массив для результатов

    for (int i = 0; i < array->size; i++) {
        int found_in_other = 0;
        for (int j = 0; j < other_array->size; j++) {
            if (array->pairs[i].type == other_array->pairs[j].type) {
                if (array->pairs[i].type == JSE_ARRAY_TYPE_INT) {
                    if (array->pairs[i].value.int_value == other_array->pairs[j].value.int_value) {
                        found_in_other = 1;
                        break;
                    }
                } else if (array->pairs[i].type == JSE_ARRAY_TYPE_STRING) {
                    if (strcmp(array->pairs[i].value.str_value, other_array->pairs[j].value.str_value) == 0) {
                        found_in_other = 1;
                        break;
                    }
                } else if (array->pairs[i].type == JSE_ARRAY_TYPE_ARRAY) {
                    // Рекурсивное сравнение для массивов
                    JSE_ARRAY nested_diff = jse_array_diff(array->pairs[i].value.array_value, other_array->pairs[j].value.array_value);
                    if (nested_diff.size > 0) {
                        found_in_other = 1;
                        jse_array_push(&diff_array, array->pairs[i].key, array->pairs[i].value, array->pairs[i].type);
                    }
                }
            }
        }
        if (!found_in_other) {
            jse_array_push(&diff_array, array->pairs[i].key, array->pairs[i].value, array->pairs[i].type);
        }
    }

    return diff_array;
}
/**
 * @brief - [JSE] [EXT] [Array] Тестирование окружения массивов
 */

void jse_array_test(){
    // Создаем два массива для сравнения
    JSE_ARRAY array1 = jse_array_create();
    jse_array_push(&array1, "age", (JSE_ARRAY_VALUE){.int_value = 30}, JSE_ARRAY_TYPE_INT);
    jse_array_push(&array1, "name", (JSE_ARRAY_VALUE){.str_value = jse_strdup("John")}, JSE_ARRAY_TYPE_STRING);


    // Первый вложенный массив
    JSE_ARRAY nested_array1 = jse_array_create();
    jse_array_push(&nested_array1, "item1", (JSE_ARRAY_VALUE){.int_value = 10}, JSE_ARRAY_TYPE_INT);
    jse_array_push(&nested_array1, "item2", (JSE_ARRAY_VALUE){.str_value = jse_strdup("nested")}, JSE_ARRAY_TYPE_STRING);
    jse_array_push(&array1, "nested", (JSE_ARRAY_VALUE){.array_value = &nested_array1}, JSE_ARRAY_TYPE_ARRAY);

    JSE_ARRAY array2 = jse_array_create();
    jse_array_push(&array2, "age", (JSE_ARRAY_VALUE){.int_value = 30}, JSE_ARRAY_TYPE_INT);
    jse_array_push(&array2, "name", (JSE_ARRAY_VALUE){.str_value = jse_strdup("Jane")}, JSE_ARRAY_TYPE_STRING);

    // Первый вложенный массив
    JSE_ARRAY nested_array2 = jse_array_create();
    jse_array_push(&nested_array2, "item1", (JSE_ARRAY_VALUE){.int_value = 10}, JSE_ARRAY_TYPE_INT);
    jse_array_push(&nested_array2, "item2", (JSE_ARRAY_VALUE){.str_value = jse_strdup("nested")}, JSE_ARRAY_TYPE_STRING);
    jse_array_push(&nested_array2, "item3", (JSE_ARRAY_VALUE){.str_value = jse_strdup("new")}, JSE_ARRAY_TYPE_STRING);
    jse_array_push(&array2, "nested", (JSE_ARRAY_VALUE){.array_value = &nested_array2}, JSE_ARRAY_TYPE_ARRAY);

    jse_array_value_dump(jse_array_getByKey(&array2, "nested"),4);
    jse_array_value_dump(jse_array_getByKey(&array2, "i5ufjk4uj"),8);

    // Выполняем сравнение массивов
    JSE_ARRAY diff = jse_array_diff(&array1, &array2);

    // Выводим результаты сравнения
    printf("Difference between array1 and array2:\n");
    jse_array_dump(diff,4);

    // Освобождаем память
    jse_array_free(&array1);
    jse_array_free(&array2);
    jse_array_free(&diff);
}


jsval_t jse_ext_array(struct js *js, jsval_t *args, int nargs) {

    qemu_log("JGA size: %d ",JSE_GLOBAL_ARRAY.size);

    // Создаем буфер
    JSE_ARRAY* arr = (JSE_ARRAY*) calloc(1, sizeof(JSE_ARRAY));
    arr->pairs = (JSE_ARRAY_DATA*) calloc(1, JSE_EXT_ARRAY_INITIAL_SIZE * sizeof(JSE_ARRAY_DATA));
    arr->size = 0;
    arr->capacity = JSE_EXT_ARRAY_INITIAL_SIZE;

    qemu_log("Created array: %x ",arr);
    jse_array_push(&JSE_GLOBAL_ARRAY, "__GLOBAL__", (JSE_ARRAY_VALUE){.array_value = arr}, JSE_ARRAY_TYPE_ARRAY);

    qemu_log("JGA size: %d ",JSE_GLOBAL_ARRAY.size);
    qemu_log("ARR size: %d ",arr->size);

    return js_mknum(JSE_GLOBAL_ARRAY.size - 1);
}

jsval_t jse_ext_array_push(struct js *js, jsval_t *args, int nargs) {
    //qemu_note("[NOTE] jse_ext_array_push");

    if (nargs < 3) return js_mkerr(js,"%n args are required.", 3);
    int Index = jse_getInt(js,args[0]);
    const char* Key = js_str(js,args[1]);
    JSE_ARRAY_DATA data = jse_array_getByIndex(&JSE_GLOBAL_ARRAY, Index);

    if (data.type != JSE_ARRAY_TYPE_ARRAY){
        return js_mkerr(js,"This element is not an array.");
    }
    //qemu_note("Array => Index: %d | Type: %d",Index, data.type);

    JSE_ARRAY_VALUE val = data.value;
    JSE_ARRAY* arr = val.array_value;

    //qemu_note("[NOTE] Test dump! Size:%d | Capacity:%d", arr->size, arr->capacity);

    int val_type = js_type(args[2]);

    if (val_type == JS_NUM){
        jse_array_push(arr, Key, (JSE_ARRAY_VALUE){.int_value = js_getnum(args[2])}, JSE_ARRAY_TYPE_INT);
    } else if (val_type == JS_STR) {
        const char* Value = js_str(js,args[2]);
        jse_array_push(arr, Key, (JSE_ARRAY_VALUE){.str_value = jse_strdup(Value)}, JSE_ARRAY_TYPE_STRING);
    } else if (val_type == JS_UNDEF || val_type == JS_FALSE || val_type == JS_NULL){
        jse_array_push(arr, Key, (JSE_ARRAY_VALUE){.int_value = 0}, JSE_ARRAY_TYPE_INT);
    } else if (val_type == JS_TRUE){
        jse_array_push(arr, Key, (JSE_ARRAY_VALUE){.int_value = 1}, JSE_ARRAY_TYPE_INT);
    } else {
        qemu_note("Undetected type: %x", val_type);
    }

    return js_mknum(arr->size - 1);
}


jsval_t jse_ext_array_editByID(struct js *js, jsval_t *args, int nargs) {
    //qemu_note("[NOTE] jse_ext_array_push");

    if (nargs < 3) return js_mkerr(js,"%n args are required.", 3);
    int Index = jse_getInt(js,args[0]);
    int Key = jse_getInt(js,args[1]);
    JSE_ARRAY_DATA data = jse_array_getByIndex(&JSE_GLOBAL_ARRAY, Index);

    if (data.type != JSE_ARRAY_TYPE_ARRAY){
        return js_mkerr(js,"This element is not an array.");
    }
    //qemu_note("Array => Index: %d | Type: %d",Index, data.type);

    JSE_ARRAY_VALUE val = data.value;
    JSE_ARRAY* arr = val.array_value;

    //qemu_note("[NOTE] Test dump! Size:%d | Capacity:%d", arr->size, arr->capacity);

    int val_type = js_type(args[2]);

    if (val_type == JS_NUM){
        jse_array_editByID(arr, Key, (JSE_ARRAY_VALUE){.int_value = js_getnum(args[2])}, JSE_ARRAY_TYPE_INT);
    } else if (val_type == JS_STR) {
        const char* Value = js_str(js,args[2]);
        jse_array_editByID(arr, Key, (JSE_ARRAY_VALUE){.str_value = jse_strdup(Value)}, JSE_ARRAY_TYPE_STRING);
    } else if (val_type == JS_UNDEF || val_type == JS_FALSE || val_type == JS_NULL){
        jse_array_editByID(arr, Key, (JSE_ARRAY_VALUE){.int_value = 0}, JSE_ARRAY_TYPE_INT);
    } else if (val_type == JS_TRUE){
        jse_array_editByID(arr, Key, (JSE_ARRAY_VALUE){.int_value = 1}, JSE_ARRAY_TYPE_INT);
    } else {
        qemu_note("Undetected type: %x", val_type);
    }

    return js_mkundef();
}


jsval_t jse_ext_array_getByKey(struct js *js, jsval_t *args, int nargs) {
    //qemu_note("[NOTE] jse_ext_array_getByKey");

    if (nargs < 2) return js_mkerr(js,"%n args are required.", 2);
    int Index = jse_getInt(js,args[0]);
    const char* Key = js_str(js,args[1]);
    JSE_ARRAY_DATA data = jse_array_getByIndex(&JSE_GLOBAL_ARRAY, Index);

    if (data.type != JSE_ARRAY_TYPE_ARRAY){
        return js_mkerr(js,"This element is not an array.");
    }
    //qemu_note("Array => Index: %d | Type: %d",Index, data.type);

    JSE_ARRAY_VALUE val = data.value;
    JSE_ARRAY* arr = val.array_value;

    //qemu_note("[NOTE] Test dump! Size:%d | Capacity:%d", arr->size, arr->capacity);

    JSE_ARRAY_DATA a_data = jse_array_getByKey(arr, Key);
    if (a_data.type == JSE_ARRAY_TYPE_NULL)   return js_mkundef();
    if (a_data.type == JSE_ARRAY_TYPE_ARRAY)  return js_mkstr(js,"(Array)",strlen("(Array)"));
    JSE_ARRAY_VALUE a_val = a_data.value;
    if (a_data.type == JSE_ARRAY_TYPE_STRING) return js_mkstr(js,a_val.str_value,strlen("a_val.str_value"));
    if (a_data.type == JSE_ARRAY_TYPE_INT) {
        char* buf = calloc(1, 64);
        itoa(a_val.int_value, buf);
        jsval_t ret = js_mkstr(js, buf, strlen(buf));
        free(buf);
        return ret;
    }
    return js_mkundef();
}


jsval_t jse_ext_array_getByIndex(struct js *js, jsval_t *args, int nargs) {
    //qemu_note("[NOTE] jse_ext_array_getByIndex");
    if (nargs < 2) return js_mkerr(js,"%n args are required.", 2);
    int Index = jse_getInt(js,args[0]);
    int Key = jse_getInt(js,args[1]);
    JSE_ARRAY_DATA data = jse_array_getByIndex(&JSE_GLOBAL_ARRAY, Index);

    if (data.type != JSE_ARRAY_TYPE_ARRAY){
        qemu_err("It's no ARRAY!");
        return js_mkfalse();
    }
   //qemu_note("Array => Index: %d | Type: %d",Index, data.type);

    JSE_ARRAY_VALUE val = data.value;
    JSE_ARRAY* arr = val.array_value;

    //qemu_note("[NOTE] Test dump! Size:%d | Capacity:%d", arr->size, arr->capacity);

    JSE_ARRAY_DATA a_data = jse_array_getByIndex(arr, Key);
    if (a_data.type == JSE_ARRAY_TYPE_NULL)   return js_mkundef();
    if (a_data.type == JSE_ARRAY_TYPE_ARRAY)  return js_mkstr(js,"(Array)",strlen("(Array)"));
    JSE_ARRAY_VALUE a_val = a_data.value;
    if (a_data.type == JSE_ARRAY_TYPE_STRING) return js_mkstr(js,a_val.str_value,strlen("a_val.str_value"));
    if (a_data.type == JSE_ARRAY_TYPE_INT) {
        char* buf = calloc(1, 64);
        itoa(a_val.int_value, buf);
        jsval_t ret = js_mkstr(js, buf, strlen(buf));
        free(buf);
        return ret;
    }
    return js_mkundef();
}

jsval_t jse_ext_array_length(struct js *js, jsval_t *args, int nargs) {
    //qemu_note("[NOTE] jse_ext_array_getByIndex");
    if (nargs < 1) return js_mkerr(js,"%n args are required.", 1);
    int Index = jse_getInt(js,args[0]);
    JSE_ARRAY_DATA data = jse_array_getByIndex(&JSE_GLOBAL_ARRAY, Index);

    if (data.type != JSE_ARRAY_TYPE_ARRAY){
        qemu_err("It's no ARRAY!");
        return js_mkfalse();
    }
    //qemu_note("Array => Index: %d | Type: %d",Index, data.type);

    JSE_ARRAY_VALUE val = data.value;
    JSE_ARRAY* arr = val.array_value;

    //qemu_note("[NOTE] Test dump! Size:%d | Capacity:%d", arr->size, arr->capacity);

    return js_mknum(arr->size);
}

jsval_t jse_ext_array_dump(struct js *js, jsval_t *args, int nargs) {
    //qemu_note("[NOTE] jse_ext_array_getByIndex");
    if (nargs < 1) return js_mkerr(js,"%n args are required.", 1);
    int Index = jse_getInt(js,args[0]);
    JSE_ARRAY_DATA data = jse_array_getByIndex(&JSE_GLOBAL_ARRAY, Index);

    if (data.type != JSE_ARRAY_TYPE_ARRAY){
        qemu_err("It's no ARRAY!");
        return js_mkfalse();
    }
    //qemu_note("Array => Index: %d | Type: %d",Index, data.type);

    JSE_ARRAY_VALUE val = data.value;
    JSE_ARRAY* arr = val.array_value;

    jse_array_link_dump(arr,0);

    return js_mkundef();
}

jsval_t jse_ext_array_diff(struct js *js, jsval_t *args, int nargs) {
    //qemu_note("[NOTE] jse_ext_array_getByIndex");
    if (nargs < 2) return js_mkerr(js,"%n args are required.", 2);
    int Index = jse_getInt(js,args[0]);
    int Index2 = jse_getInt(js,args[1]);
    JSE_ARRAY_DATA data = jse_array_getByIndex(&JSE_GLOBAL_ARRAY, Index);

    if (data.type != JSE_ARRAY_TYPE_ARRAY){
        qemu_err("#1 It's no ARRAY!");
        return js_mkfalse();
    }

    JSE_ARRAY_DATA data2 = jse_array_getByIndex(&JSE_GLOBAL_ARRAY, Index2);

    if (data2.type != JSE_ARRAY_TYPE_ARRAY){
        qemu_err("#2 It's no ARRAY!");
        return js_mkfalse();
    }

    JSE_ARRAY_VALUE val1 = data.value;
    JSE_ARRAY* array = val1.array_value;

    JSE_ARRAY_VALUE val2 = data2.value;
    JSE_ARRAY* other_array = val2.array_value;

    ////////////////////////

    // Создаем буфер
    JSE_ARRAY* diff_array = (JSE_ARRAY*)calloc(1, sizeof(JSE_ARRAY));
    diff_array->pairs = (JSE_ARRAY_DATA*)calloc(1, JSE_EXT_ARRAY_INITIAL_SIZE * sizeof(JSE_ARRAY_DATA));
    diff_array->size = 0;
    diff_array->capacity = JSE_EXT_ARRAY_INITIAL_SIZE;

    qemu_log("Created array: %x ",diff_array);
    jse_array_push(&JSE_GLOBAL_ARRAY, "__GLOBAL__", (JSE_ARRAY_VALUE){.array_value = diff_array}, JSE_ARRAY_TYPE_ARRAY);

    for (int i = 0; i < array->size; i++) {
        int found_in_other = 0;
        for (int j = 0; j < other_array->size; j++) {
            if (array->pairs[i].type == other_array->pairs[j].type) {
                if (array->pairs[i].type == JSE_ARRAY_TYPE_INT) {
                    if (array->pairs[i].value.int_value == other_array->pairs[j].value.int_value) {
                        found_in_other = 1;
                        break;
                    }
                } else if (array->pairs[i].type == JSE_ARRAY_TYPE_STRING) {
                    if (strcmp(array->pairs[i].value.str_value, other_array->pairs[j].value.str_value) == 0) {
                        found_in_other = 1;
                        break;
                    }
                } else if (array->pairs[i].type == JSE_ARRAY_TYPE_ARRAY) {
                    // Рекурсивное сравнение для массивов
                    JSE_ARRAY nested_diff = jse_array_diff(array->pairs[i].value.array_value, other_array->pairs[j].value.array_value);
                    if (nested_diff.size > 0) {
                        found_in_other = 1;
                        jse_array_push(&diff_array, array->pairs[i].key, array->pairs[i].value, array->pairs[i].type);
                    }
                }
            }
        }
        if (!found_in_other) {
            jse_array_push(diff_array, array->pairs[i].key, array->pairs[i].value, array->pairs[i].type);
        }
    }

    ///////////////////////

    return js_mknum(JSE_GLOBAL_ARRAY.size - 1);
}

/**
 * @brief - [JSE] [EXT] [Array] Настройка окружения для функционала JSE | Поддержка массивов
 *
 * @param struct js* js - Ссылка на JSE
 */
void jse_array_config(struct js* js){
    qemu_note("[JSE] [EXT] [Array] Registration of functions");
    JSE_GLOBAL_ARRAY = jse_array_create();
    js_set(js, js_glob(js), "array", js_mkfun(jse_ext_array));
    js_set(js, js_glob(js), "array_push", js_mkfun(jse_ext_array_push));
    js_set(js, js_glob(js), "array_getByKey", js_mkfun(jse_ext_array_getByKey));
    js_set(js, js_glob(js), "array_getByIndex", js_mkfun(jse_ext_array_getByIndex));
    js_set(js, js_glob(js), "array_length", js_mkfun(jse_ext_array_length));
    js_set(js, js_glob(js), "array_dump", js_mkfun(jse_ext_array_dump));
    js_set(js, js_glob(js), "array_diff", js_mkfun(jse_ext_array_diff));
    js_set(js, js_glob(js), "array_editByID", js_mkfun(jse_ext_array_editByID));

    ///jse_array_test();
}

void jse_array_destroZ(JSE_ARRAY* array, int space){
    char* sp = (char*)calloc(1, sizeof(char) * (space + 1));
    memset(sp,' ', space);
    sp[space] = 0;

    qemu_note("%s|--- Size:%d | Capacity:%d", sp, array->size, array->capacity);
    for (int i = 0; i < array->size; i++){
        qemu_note("%s|--- Delete data %d / %d",sp, i + 1, array->size);
        if (array->pairs[i].type != JSE_ARRAY_TYPE_STRING &&
            array->pairs[i].type != JSE_ARRAY_TYPE_INT &&
            array->pairs[i].type != JSE_ARRAY_TYPE_ARRAY){
            qemu_log(" %s   |--- Skipping data (Fantom type: %x)", sp, array->pairs[i].type);
            continue;
        }
        free(array->pairs[i].key);
        qemu_ok("  %s   |--- Delete key",sp);
        if (array->pairs[i].type == JSE_ARRAY_TYPE_STRING) {
            free(array->pairs[i].value.str_value);
            qemu_ok("  %s   |--- Delete string",sp);
        } else if (array->pairs[i].type == JSE_ARRAY_TYPE_ARRAY) {
            // Дополнительная логика для освобождения памяти массива
            // Игнорируется, тк данный участок будет позже отчищен автоматический
            qemu_note("%s   |--- [IGN] GOTO %x",sp , array->pairs[i].value.array_value);
            //jse_array_destroZ(array->pairs[i].value.array_value, sp + 8);
        }
    }
    qemu_ok("  %s|--- Delete pars",sp);
    free(array->pairs);
    qemu_ok("  %s|--- Delete array",sp);
    free(array);
    free(sp);
}

void jse_array_destroy(struct js* js){
    //qemu_err("[JSE] [EXT] [Array] [Destroy] DISABLED!!!!!");
    //return;
    qemu_note("[JSE] [EXT] [Array] Destroy");

    qemu_note("  |--- Size:%d | Capacity:%d", JSE_GLOBAL_ARRAY.size, JSE_GLOBAL_ARRAY.capacity);
    for (int i = 0; i < JSE_GLOBAL_ARRAY.size; i++){
        qemu_note("  |--- Delete data %d / %d", i + 1, JSE_GLOBAL_ARRAY.size);
        free(JSE_GLOBAL_ARRAY.pairs[i].key);
        qemu_ok("       |--- Delete key");
        if (JSE_GLOBAL_ARRAY.pairs[i].type == JSE_ARRAY_TYPE_STRING) {
            free(JSE_GLOBAL_ARRAY.pairs[i].value.str_value);
            qemu_ok("     |--- Delete string");
        } else if (JSE_GLOBAL_ARRAY.pairs[i].type == JSE_ARRAY_TYPE_ARRAY) {
            // Дополнительная логика для освобождения памяти массива
            qemu_note("     |--- GOTO %x", JSE_GLOBAL_ARRAY.pairs[i].value.array_value);
            jse_array_destroZ(JSE_GLOBAL_ARRAY.pairs[i].value.array_value, 8);
        }
    }
    qemu_ok("    |--- Delete global pars");
    free(JSE_GLOBAL_ARRAY.pairs);
    qemu_ok("    |--- Delete global array");
    //free(JSE_GLOBAL_ARRAY);
}