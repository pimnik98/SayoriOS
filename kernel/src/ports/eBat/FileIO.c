#include "kernel.h"
#include "eBat.h"
#include "eBatRuntime.h"

/**
 * Проверка наличии файла или папки
 * @module FileIO.Exits
 * @param path Путь к файлу
 * @return 1 - Если файл найден или 0 - если нет
 */
int bat_runtime_fileio_exist(char* path){
    /// Insert your code
    bat_debug("[RUNTIME] [FileIO] [Exits] %s\n", path);
    bool ret = file_exists(path);
    return ret;
}

int bat_runtime_fileio_write(const char *filename, const char *text, int mode) {
    /// Insert your code
    return 0;
}