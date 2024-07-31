#include "kernel.h"
#include "eBat.h"
#include "eBatRuntime.h"

int bat_strtol(char *string) {
    int number = 0;
    int sign = 1;
    int i = 0;

    // Пропускаем пробелы в начале строки
    while (string[i] == ' ' && string[i] != '\0') {
        i++;
    }

    // Обработка знака
    if (string[i] == '-' || string[i] == '+') {
        sign = (string[i] == '-' ? -1 : 1);
        i++;
    }

    // Извлечение цифр
    while (isdigit(string[i]) && string[i] != '\0') {
        number = number * 10 + (string[i] - '0');
        i++;
    }

    return number * sign;
}
void bat_trim(char* string){
    int start = 0, end = strlen(string) - 1;
    while (string[start] == ' ' || string[start] == '\t' || string[start] == '\n' || string[start] == 0xD){
        start++;
    }
    while (string[end] == ' ' || string[end] == '\t' || string[end] == '\n' || string[end] == 0xD){
        end--;
    }
    for (int i = 0; i <= end - start; i++){
        string[i] = string[i + start];
    }
    string[end - start + 1] = '\0';
}

void bat_str_debug(char* string){
    return;
    bat_debug("      |--- String debug: '%s'\n", string);
    int len = strlen(string);
    for (int i = 0; i < len; i++){
        bat_debug("        |--- [%d | %d] [0x%x] '%c'\n", i+1, len, string[i], string[i]);
    }
}

char* bat_strdup(const char *str) {
    // Проверяем, не является ли указатель NULL
    if (str == NULL) {
        return NULL;
    }

    // Получаем длину исходной строки
    size_t len = strlen(str);

    // Выделяем память для новой строки (включая один байт для нулевого терминатора)
    char *copy = (char*)malloc(len + 1);
    memset(copy, 0, len + 1);
    if (copy == NULL) {
        return NULL; // Проверка на успешное выделение памяти
    }

    // Копируем строку в новую память
    strcpy(copy, str);

    return copy; // Возвращаем указатель на новую строку
}

char* bat_toLower(char* str) {
    int i;
    for (i = 0; str[i] != '\0'; i++) {
        if (str[i] >= 'A' && str[i] <= 'Z') { // Проверяем, является ли символ заглавным
            str[i] = str[i] + 32; // Преобразуем в строчную букву
        }
    }
    return str; // Возвращаем изменённую строку
}

char* bat_toUpper(char *str) {
    int i;
    for (i = 0; str[i] != '\0'; i++) {
        if (str[i] >= 'a' && str[i] <= 'z') {
            str[i] = str[i] - 32;
        }
    }
    return str;
}