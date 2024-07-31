#include "kernel.h"
#include "eBat.h"
#include "eBatRuntime.h"

int bar_runtime_system_exec(int argc, char** argv){
    /// Insert your code to execute the "echo" command
    bat_debug("[RUNTIME] [System] [EXEC] Count: %d\n", argc);
    int ret = cli_handler_ebat(argc, argv);
    return ret;
}

/**
 * Вывод текста на экран
 * @module System.Echo
 * @param text - Строка для вывода текст
 */
void bat_runtime_system_echo(char* text, int newline, int endline){
    /// Insert your code to execute the "echo" command
    bat_debug("[RUNTIME] [System] [ECHO] %s\n", text);
    if (text == NULL){
        return;
    }

    //printf("%s%s%s", (newline == 1?"< ":""), text, (endline == 1?" \n":" "));
    tty_printf("%s%s%s", (newline == 1?"< ":""), text, (endline == 1?" \n":" "));
}

/**
 * Установка значения переменной
 * @param key - Ключ
 * @param val - Значение
 */
void bat_runtime_system_set(char* key, char* val){
    /// Insert your code
    //bat_debug("[RUNTIME] [System] [SET] '%s' => '%s'\n", key, val);
    //printf("[RUNTIME] [System] [SET] '%s' => '%s'\n", key, val);
    //bat_toUpper(key);
    //char* get = bat_runtime_system_get(key);

    int len_key = (key == NULL?0:strlen(key));
    int len_val = (val == NULL?0:strlen(val));

    if (key == NULL){
        return;
    }
    if (val == NULL || len_val == 0){
        variable_write(key,"");
    }
}

/**
 * Получение значения переменной
 * @param key - Ключ
 */
char* bat_runtime_system_get(char* key){
    /// Insert your code
    bat_debug("[RUNTIME] [System] [GET] '%s'\n", key);
    //bat_toUpper(key);
    return variable_read(key);
}


void bat_runtime_system_pause(){
    /// Insert your code
    bat_debug("[RUNTIME] [System] [Pause]\n");
    bat_runtime_system_echo("Please, press button", 1, 1);
    getIntKeyboardWait();
    ///getchar(); - Функция которая ожидает ввода любой клавиши
}