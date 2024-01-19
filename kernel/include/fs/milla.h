/**
 *
 * @file fs/milla.c
 * @author Пиминов Никита (nikita.piminoff@yandex.ru), NDRAEY >_ (pikachu_andrey@vk.com)
 * @brief Проект Милла
 * @version 0.3.5
 * @date 2023-01-23
 * @copyright Copyright SayoriOS Team (c) 2022-2024
 */

void __milla_sendcmd(char* msg);
char* __milla_getcmd();
int __milla_cleanState();
char* __milla_getFile(const char *path);
int __milla_writeFile(char* path, char* data);
int __milla_delete(char* path);
int __milla_mkdir(char* path);
int __milla_touch(char* path);
char* __milla_getList(char* path);
int __milla_init();
void __milla_destroy();
