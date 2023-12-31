/// JavaScript Engine - Поддержка библиотек
#include "portability.h"
#include <io/ports.h>


#include "io/tty.h"
#include "lib/libstring/string.h"
#include "../libvector/include/vector.h"
#include "lib/stdio.h"


#include "elk_config.h"
#include "elk.h"



void jse_file_config(const char *cfg, const char *file){
    size_t minStack = JSE_MIN_STACK;
    size_t bufferSize = 0;
    size_t stack = JSE_MIN_STACK;
    bool jse_p_no_warn_stack = false;
    size_t param_size = -1;
    char* out = kcalloc(1,sizeof(char) * minStack + bufferSize);
    string_t* str = string_from_charptr(cfg);
    vector_t* vec = string_split(str, "\n");

    for(int i = 0; i < vec->size; i++) {
        param_size++;
        string_t* str_param = (string_t*)(vec->data[i]);
        vector_t* param = string_split(str_param, " ");
        //qemu_log("[VEC] [%d/%d] [C:%d] Str: %s", i, vec->size, param->size, str_param->data);

        if (param->size < 2) continue;

        jse_trim(((string_t*)(param->data[0]))->data);
        jse_trim(((string_t*)(param->data[1]))->data);
        //qemu_note("Key: '%s' | Value: '%s'", ((string_t*)(param->data[0]))->data, ((string_t*)(param->data[1]))->data);
        if (strcmpn(((string_t*)(param->data[0]))->data,"//#include")){
            char* fn_buf = kcalloc(1,sizeof(char) * (strlen(JSE_LIBS_PATH) + strlen(((string_t*)(param->data[1]))->data) + 1 ) );

            memcpy(fn_buf, JSE_LIBS_PATH, strlen(JSE_LIBS_PATH));
            memcpy(fn_buf + strlen(JSE_LIBS_PATH), (((string_t*)(param->data[1]))->data), strlen(((string_t*)(param->data[1]))->data));

            size_t lastInx = strlen(JSE_LIBS_PATH) + strlen(((string_t*)(param->data[1]))->data);

            fn_buf[lastInx] = 0;

            FILE* file = fopen(fn_buf, "r");
            if (!file){
                qemu_err("[JSE] [Include] File '%s' no found!", fn_buf);
                kfree(fn_buf);
                fclose(file);
            } else {

                size_t filesize = fsize(file);
                bufferSize += filesize;

                char* buf = kcalloc(1,sizeof(char) * (filesize+1));
                fread(file, 1, filesize, buf);

                out = jse_mergeBuffers(out, buf, strlen(out), filesize);

                fclose(file);
                kfree(buf);
                kfree(fn_buf);

                qemu_ok("   [JSE] [Include] File '%s' is loaded!", fn_buf);


            }
        } else if (param->size <= 3 && strcmpn(((string_t*)(param->data[0]))->data,"//#pragma")) {
            jse_trim(((string_t*)(param->data[2]))->data);
            qemu_note(" [JSE] [Pragma] key: '%s' value: '%s'", ((string_t*)(param->data[1]))->data, ((string_t*)(param->data[2]))->data);
            if (strcmpn(((string_t*)(param->data[1]))->data,"stack")){
                int stackValue = jse_func_atoi(((string_t*)(param->data[2]))->data);
                if (stackValue < stack){
                    if (!jse_p_no_warn_stack) qemu_warn(" [JSE] [WARN] The stack size has been reset to the minimum size by the installed JSE.");
                    continue;
                } else if (stackValue > (strlen(file) + bufferSize) * 5) {
                    if (!jse_p_no_warn_stack) qemu_warn(" [JSE] [WARN] You have set the stack value to less than the recommended value (%d), errors may appear during script execution.", (strlen(file) + bufferSize) * 5);
                }
                stack = stackValue;
                continue;
            } else if (strcmpn(((string_t*)(param->data[1]))->data,"no-warn-stack")){
                int value = jse_func_atoi(((string_t*)(param->data[2]))->data);
                jse_p_no_warn_stack = (value == 1?true:false);
                continue;
            }
        } else {
            qemu_err("[JSE] Unknown argv (%s)", ((string_t*)(param->data[0]))->data);
            continue;
        }
        //printf("%s\n", ((string_t*)(vec->data[i]))->data);
    }


    string_split_free(vec);
    string_destroy(str);

    out = jse_mergeBuffers(out, file, strlen(out), strlen(file));
    qemu_note(" [JSE] Stack size: %d", stack);
    stack += sizeof(struct js) + 1;

    /// JSE Runtime


    char* js_mem = kcalloc(1,stack);
    struct js *js = js_create(js_mem, (stack));

    elk_setup(js);

    js->incSize = bufferSize;
    js->paramSize = param_size;

    //qemu_note("[Buffer  PRE] [%d] \n=============\n%s\n=============\n", strlen(out), out);

    //jse_checkBuffer(out);

    //qemu_note("[Buffer POST] [%d] \n=============\n%s\n=============\n", strlen(out), out);


    jsval_t result = js_eval(js, out, strlen(out));

    elk_destroy(js);

    if (js->isFatal){
        qemu_err("\n[JSE] Runtime Fatal Error!\n\r      Message: %s\n", js_str(js, result));
    } else {
        printf("[JSE] Result: %s\n",js_str(js, result));
    }

    js_statsInfo(js);


    kfree(out);
    kfree(js_mem);
    //kfree(file);
    //return (js->isFatal == 1?0:1);

    //qemu_note("Buffer: \n%s", out);
}

void jse_file_preconfig(const char *buffer, const char *first_search, const char *second_search) {
    const char *start = buffer;
    const char *first_occurrence = jse_strstr(start, first_search);

    if (first_occurrence != NULL) {
        start = first_occurrence + strlen(first_search);
        const char *second_occurrence = jse_strstr(start, second_search);

        if (second_occurrence != NULL) {
            int content_length = second_occurrence - start;

            int fileoff = content_length + strlen(first_search) + strlen(second_search);

            void* config = calloc(sizeof(char) * (content_length + 1), 1);
            void* file = calloc(sizeof(char) * (strlen(buffer) - content_length), 1);

            jse_ncpy((char*) config, (char*) start, content_length);
            jse_ncpy((char*) file, (char*) buffer + fileoff, (strlen(buffer) - content_length));

            jse_file_config((char*) config, (char*) file);

            return;
        }
    }

    qemu_note("[JSE] The JSE configuration block was not found in the file, so JSE executes in normal mode and with standard settings.");
    elk_eval(buffer);
}

void jse_file_getBuff(char* buf){
    const char *sc1 = "//<#JSE#";
    const char *sc2 = "//#JSE#>";

    jse_file_preconfig(buf, sc1, sc2);
}
