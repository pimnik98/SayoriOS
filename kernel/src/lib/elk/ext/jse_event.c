/// JavaScript Engine - Поддержка событый
#include "portability.h"
#include <io/ports.h>
#include "io/tty.h"
#include "lib/stdio.h"


#include <drv/input/keyboard.h>
#include <drv/input/mouse.h>

#include "../elk_config.h"
#include "../elk.h"

int jse_event_kbd_i = -1;        ///< Индекс триггера клавиатуры
int jse_event_mouse_i = -1;      ///< Индекс триггера мыши

JSE_EVENT_KBD_KEY_STATE jse_event_kbd_state[256]= {0}; // Массив для отслеживания состояния каждой клавиши
uint32_t jse_event_kbd_combo = 0;
uint32_t jse_event_kbd_lastCode = 0;
uint32_t jse_event_kbd_lastTime = 0;


uint32_t jse_event_mouse_lastTime = 0;
uint32_t jse_event_mouse_x = 0;
uint32_t jse_event_mouse_y = 0;
uint32_t jse_event_mouse_combo = 0;


void jse_event_mouse(void* data1,void* data2,void* data3,void* data4,void* data5){
    jse_event_mouse_x = (int) data4;
    jse_event_mouse_y = (int) data5;
    jse_event_mouse_combo = ((int) data1 << 2) | ((int) data2 << 1) | (int) data3;
}

void jse_event_kbd(void* data1,void* data2,void* data3,void* data4,void* data5){
    int keyCode = (int)data1;
    int keyPress = (int)data2;
    ///qemu_log("[JSE] [Event] [KBD] Key: %d | Press: %d",keyCode, keyPress);
    jse_event_kbd_lastCode = (keyPress?keyCode:0);
    uint32_t jse_event_kbd_lastTime = 0;
    jse_event_kbd_state[keyCode].Last = getTicks();       // Сохраняем время последнего вызова
    // Обновляем состояние клавиши
    if (keyPress) { // Клавиша зажата
        if (jse_event_kbd_state[keyCode].Status == 0) {       // Клавиша была отпущена
            jse_event_kbd_state[keyCode].Start = getTicks();      // Устанавливаем время нажатия
            jse_event_kbd_state[keyCode].End = 0;
            jse_event_kbd_state[keyCode].Status = 1;              // Обновляем статус клавиши
            jse_event_kbd_combo |= (1 << keyCode);                // Установка комбо-значения
        } else {
            return;
        }
    } else { // Клавиша отпущена
        if (jse_event_kbd_state[keyCode].Status == 1) {       // Клавиша была зажата
            jse_event_kbd_state[keyCode].End = getTicks();        // Устанавливаем время отпускания
            jse_event_kbd_state[keyCode].Status = 0;              // Обновляем статус клавиши
            jse_event_kbd_state[keyCode].Last = getTicks();       // Сохраняем время последнего вызова
            jse_event_kbd_combo &= ~(1 << keyCode);               // Снятие комбо-значения
        }
    }
}



jsval_t jse_ext_event_test1(struct js *js, jsval_t *args, int nargs) {
    int x = 0x1000;  // Fetch 1st arg
    char* test = "Test elly net!";
    qemu_log(" [JSE] [EXT] [Event] [test1] x:%x", test);
    return js_mkstr(js, test, strlen(test));
}

jsval_t jse_ext_event_kbd_waitForAnyKey(struct js *js, jsval_t *args, int nargs) {
    if (nargs == 0) return js_mknum(0);
    while (1){
        for (int i = 0; i < nargs; i++){
            int c = jse_getInt(js,args[i]);
            if (jse_event_kbd_lastCode == c){
                return js_mknum(c);
            }
        }
    }
    return js_mktrue();
}

jsval_t jse_ext_event_kbd_waitForAnyKeyOnce(struct js *js, jsval_t *args, int nargs) {
    if (nargs == 0) return js_mknum(0);
    while (1){
        for (int i = 0; i < nargs; i++){
            int c = jse_getInt(js,args[i]);
            if (jse_event_kbd_lastCode == c){
                jse_event_kbd_lastCode = 0;
                return js_mknum(c);
            }
        }
    }
    return js_mktrue();
}

jsval_t jse_ext_event_kbd(struct js *js, jsval_t *args, int nargs) {
    if (nargs != 1) return js_mknum(jse_event_kbd_lastCode);
    int c = jse_getInt(js,args[0]);
    if (c == 0) return js_mknum(jse_event_kbd_lastCode);
    if (c == 1) return js_mknum(jse_event_kbd_lastTime);
    if (c == 2) return js_mknum(jse_event_kbd_combo);
    return js_mkundef();
}


jsval_t jse_ext_event_mouse(struct js *js, jsval_t *args, int nargs) {
    if (nargs != 1) return js_mknum(jse_event_mouse_combo);
    int c = jse_getInt(js,args[0]);
    if (c == 0) return js_mknum(jse_event_mouse_combo);
    if (c == 1) return js_mknum(jse_event_mouse_x);
    if (c == 2) return js_mknum(jse_event_mouse_y);
    if (c == 3) return js_mknum(jse_event_mouse_lastTime);
    return js_mkundef();
}

jsval_t jse_ext_event_debug(struct js *js, jsval_t *args, int nargs) {
    if (nargs != 1) return js_mkundef();
    int t = js_type(args[0]);
    int x = jse_getInt(js,args[0]);

    const char* str = js_str(js,args[0]);
    qemu_log(" [JSE] [EXT] [Event] [Debug] T:%d | X:%x | (%d) %s", t, x, strlen(str), str);

    if (t == JS_PRIV){
        // Это тип данных объекта
        JSE_ARRAY* arr = js_getObjectToArray(js, args[0]);
        //jse_array_editByID(arr, 2, (JSE_ARRAY_VALUE){.int_value = 5730575}, JSE_ARRAY_TYPE_INT);
        jse_array_link_dump(arr, 0);
        jse_array_destroZ(arr,0);
        jse_array_free(arr);
    }

    return js_mktrue();
}

void jse_event_config(struct js* js){
    qemu_note("[JSE] [EXT] [Event] Registration of functions");
    js_set(js, js_glob(js), "event_debug", js_mkfun(jse_ext_event_debug));
    js_set(js, js_glob(js), "event_test1", js_mkfun(jse_ext_event_test1));
    js_set(js, js_glob(js), "kbd", js_mkfun(jse_ext_event_kbd));
    js_set(js, js_glob(js), "kbd_waitForAnyKey", js_mkfun(jse_ext_event_kbd_waitForAnyKey));
    js_set(js, js_glob(js), "kbd_waitForAnyKeyOnce", js_mkfun(jse_ext_event_kbd_waitForAnyKeyOnce));
    js_set(js, js_glob(js), "mouse", js_mkfun(jse_ext_event_mouse));

    if (jse_event_kbd_i == -1){
        jse_event_kbd_i = RegTrigger(0x0001, &jse_event_kbd);	    ///< Регистрация нажатий клавы
    } else {
        OnTrigger(jse_event_kbd_i);
    }

    if (jse_event_mouse_i == -1){
        jse_event_mouse_i = RegTrigger(0x0003, &jse_event_mouse);	///< Регистрация нажатий и движений мыши
    } else {
        OnTrigger(jse_event_mouse_i);
    }

    /// Отключаем анимацию и вывод ввода клавиатуры
    keyboardctl(KEYBOARD_ECHO, false);
    set_cursor_enabled(false);
}

void jse_event_destroy(struct js* js){
    qemu_note("[JSE] [EXT] [Event] Destroy");
    if (jse_event_kbd_i != -1){
        OffTrigger(jse_event_kbd_i);
    }

    if (jse_event_mouse_i != -1){
        OffTrigger(jse_event_mouse_i);
    }

    /// Возвращаем анимацию и вывод ввода клавиатуры
    keyboardctl(KEYBOARD_ECHO, true);
    set_cursor_enabled(true);
}
