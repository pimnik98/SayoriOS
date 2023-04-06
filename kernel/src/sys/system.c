/**
 * @file sys/system.c
 * @author Пиминов Никита (nikita.piminoff@yandex.ru)
 * @brief Дополнительные системные функции
 * @version 0.3.2
 * @date 2022-10-01
 * @copyright Copyright SayoriOS Team (c) 2022-2023
 */
#include <kernel.h>

// FIXME: These variables gets rewritten to address 0 when
//        user types 'cd /' in shell
char* whoami    = "root";           ///< Имя пользователя
char* hostname  = "oem";            ///< Имя устройства
char* syspath   = "/";              ///< Путь по умолчанию
/**
 * @brief Получить текущий путь
 *
 * @return char* путь
 */
char* getSysPath(){
    return syspath;
}

/**
 * @brief Установить текущий путь
 *
 * @param char* path - путь
 *
 * @return char* путь
 */
void setSysPath(char* path){
    kfree(syspath);
    
    syspath = kmalloc(sizeof(char) * (strlen(path) + 1));
    
    memset(syspath,0,strlen(path)+1);
    memcpy(syspath,path,strlen(path));
    
    // syspath[strlen(path)-1] = 0;
}

/**
 * @brief Перезагрузка устройства
 */
void reboot() {
    qemu_log("REBOOT");

    uint8_t good = 0x02;

    while (good & 0x02)
        good = inb(0x64);
    outb(0x64, 0xFE);
    
    asm volatile("hlt");
}

/**
 * @brief Выключение устройства
 */
void shutdown(){
    qemu_log("SHUTDOWN");
    outw(0xB004, 0x2000);
    outw(0x604,  0x2000);
    outw(0x4004, 0x3400);
}

/**
 * @brief Возращает имя пользователя
 *
 * @return char* - Имя пользователя
 */
char* getUserName(){
    return whoami;
}

/**
 * @brief Устанавливает имя пользователя
 *
 * @param char* new - Новое имя пользователя
 *
 * @return char* - Имя пользователя
 */
char* setUserName(char* new){
    kfree(whoami);
    
    whoami = kmalloc(sizeof(char) * (strlen(new) + 1));
    
    memset(whoami, 0, strlen(new)+1);
    memcpy(whoami, new, strlen(new));
    
    return whoami;
}

/**
 * @brief Возращает имя устройства
 *
 * @return char* - Имя устройства
 */
char* getHostname(){
    return hostname;
}

/**
 * @brief Устанавливает имя устройства
 *
 * @param char* new - Новое имя устройства
 *
 * @return char* - Имя устройства
 */
char* setHostname(char* new){
    if (strlen(new) < 2){
        tty_printf("[ОШИБКА] Имя устройства должно быть больше 2 символов");
        return hostname;
    }
    
    kfree(hostname);

    hostname = kmalloc(sizeof(char)*(strlen(new)+1));
    
    memset(hostname,0,strlen(new)+1);
    memcpy(hostname,new,strlen(new));
    
    // hostname[strlen(new)-1] = 0;
    return hostname;
}
