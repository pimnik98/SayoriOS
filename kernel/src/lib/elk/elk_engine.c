/// JavaScript Engine
#include "portability.h"
#include <io/ports.h>
#include "elk.h"
#include "io/tty.h"
#include "lib/stdio.h"


jsval_t jse_fnc_system_ticks(struct js *js, jsval_t *args, int nargs) {
    return js_mknum(getTicks());
}

jsval_t js_drawRect(struct js *js, jsval_t *args, int nargs) {
    if (nargs != 5) return js_mkundef();
    int x = jse_getInt(js,args[0]);  // Fetch 1st arg
    int y = jse_getInt(js,args[1]);  // Fetch 2nd arg
    int w = jse_getInt(js,args[2]);  // Fetch 3nd arg
    int h = jse_getInt(js,args[3]);  // Fetch 4nd arg
    int color = jse_getInt(js,args[4]);  // Fetch 5nd arg
    //qemu_warn("x:%d | y:%d | w:%d | h:%d | color: %x",x,y,w,h,color);
    drawRect(x,y,w,h,color);
    return js_mktrue();
}

static jsval_t js_console_debug(struct js *js, jsval_t *args, int nargs) {
    for (int i = 0; i < nargs; i++){
        int type = js_type(args[i]);

        printf("[Index: %d] [Type:%d] %s\n", i, type, js_str(js,args[i]));
    }
    return js_mkundef();
}

static jsval_t js_console_log(struct js *js, jsval_t *args, int nargs) {
    for (int i = 0; i < nargs; i++){
        qemu_log("%s", js_str(js,args[i]));
    }
    return js_mkundef();
}

static jsval_t js_console_note(struct js *js, jsval_t *args, int nargs) {
    for (int i = 0; i < nargs; i++){
        qemu_note("%s", js_str(js, args[i]));
    }
    return js_mkundef();
}

static jsval_t js_console_warn(struct js *js, jsval_t *args, int nargs) {
    for (int i = 0; i < nargs; i++){
        qemu_warn("%s", js_str(js,args[i]));
    }
    return js_mkundef();
}

static jsval_t js_console_err(struct js *js, jsval_t *args, int nargs) {
    for (int i = 0; i < nargs; i++){
        qemu_err("%s", js_str(js,args[i]));
    }
    return js_mkundef();
}

static jsval_t js_print(struct js *js, jsval_t *args, int nargs) {
    for (int i = 0; i < nargs; i++){
        tty_printf("%s", js_str(js,args[i]));
    }
    return js_mkundef();
}

static jsval_t js_alert(struct js *js, jsval_t *args, int nargs) {
    for (int i = 0; i < nargs; i++){
        const char* af = js_str(js,args[i]);
        printf("%s", af);
    }
    printf("\n");
    return js_mkundef();
}

static jsval_t js_nop(struct js *js, jsval_t *args, int nargs) {
    return js_mkundef();
}

void elk_destroy(struct js* js){
    jse_array_destroy(js);
    jse_event_destroy(js);
    jse_canvas_destroy(js);
}

void elk_setup(struct js* js){
    /// Console Func
    js_set(js, js_glob(js), "console_debug", js_mkfun(js_console_debug));
    js_set(js, js_glob(js), "console_log", js_mkfun(js_console_log));
    js_set(js, js_glob(js), "console_warn", js_mkfun(js_console_warn));
    js_set(js, js_glob(js), "console_err", js_mkfun(js_console_err));
    js_set(js, js_glob(js), "console_note", js_mkfun(js_console_note));
    js_set(js, js_glob(js), "nop", js_mkfun(js_nop));

    /// System Func
    js_set(js, js_glob(js), "system_ticks", js_mkfun(jse_fnc_system_ticks));

    /// Configurate canvas
    jse_canvas_config(js);

    /// Configurate event
    jse_event_config(js);

    /// Configurate Array
    jse_array_config(js);

    /// Other Func
    js_set(js, js_glob(js), "rect", js_mkfun(js_drawRect));
    js_set(js, js_glob(js), "print", js_mkfun(js_print));
    js_set(js, js_glob(js), "alert", js_mkfun(js_alert));
}

int elk_eval(const char* buf){
    size_t stack = 32768 + sizeof(struct js);

    char* js_mem = kcalloc(stack, 1);
//    char js_mem[32768] = {0};
    struct js *js = js_create(js_mem, (stack));
    if (js == NULL){
        qemu_err("\n[JSE] Runtime Fatal Error!\n\r      Message: %s\n","No free RAM");
        return 0;
    }

    elk_setup(js);

    jsval_t result = js_eval(js, buf, strlen(buf));

    elk_destroy(js);

    if (js->isFatal){
        printf("\n[JSE] Runtime Fatal Error!\n\r      Message: %s\n",js_str(js, result));
    } else {
        printf("[JSE] Result: %s\n",js_str(js, result));
    }

    js_statsInfo(js);
    kfree(js_mem);
    return (js->isFatal == 1?0:1);
}

int elk_file(const char* path){
    FILE* file = fopen(path, "r");
    if (!file){
        qemu_err("[JSE] File no found!");
        return 0;
    }

    size_t filesize = fsize(file);

    char* buf = kcalloc(sizeof(char) * (filesize+1), 1);
    fread(file, 1, filesize, buf);

    jse_file_getBuff(buf);

    fclose(file);
    kfree(buf);
    return 1;

}