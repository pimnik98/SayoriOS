/**
 * @file devmgr_cli.c
 * @author Никита Пиминов (github.com/pimnik98)
 * @brief Менеджер устройств CLI
 * @version 0.0.3
 * @date 2022-09-18
 * @copyright Copyright SayoriOS Team
 */
#include <kernel.h>
#include <drivers/devmgr.h>
/**
 * @brief DevMgr - Вывод справки
 * @warning Функция сделана для консоли
 */
void devmgr_cli_help(){
    tty_printf("|---КОМАНДЫ-----------------------------------------------------------|\n");
    tty_printf("| -> devmgr help                        | Отображает это меню справки |\n");
    tty_printf("| -> devmgr reg [VendorID] [DeviceID]   | Регистрация устройства      |\n");
    tty_printf("| -> devmgr info [ID]                   | Информация об устройстве    |\n");
    tty_printf("| -> devmgr vendor [ID]                 | Информация об поставщике    |\n");
    tty_printf("|---ПРИМЕРЫ-----------------------------------------------------------|\n");
    tty_printf("| -> devmgr info 1                      | Выведет информацию об       |\n");
    tty_printf("|                                       | устройстве, которое         |\n");
    tty_printf("|                                       | подключено к индексу 1      |\n");
    tty_printf("|                                       |                             |\n");
    tty_printf("| -> devmgr reg 4660 4369               | Подключит к системе уст-во  |\n");
    tty_printf("|                                       | с поставщиком 0x1234 и      |\n");
    tty_printf("|                                       | устройством 0x1111          |\n");
    tty_printf("|                                       |                             |\n");
    tty_printf("| -> devmgr vendor 32902                | Покажет название поставщика |\n");
    tty_printf("|---------------------------------------------------------------------|\n");
}

/**
 * @brief DevMgr - Регистрация устройства
 * @warning Функция сделана для консоли
 *
 * @param uint32_t VendorID - ID Поставщика
 * @param uint32_t DeviceID - ID Устройства
 */
void devmgr_cli_reg(uint32_t VendorID,uint32_t DeviceID){
    uint32_t i = registerDevice(VendorID,DeviceID);
    if (i == -1){
        tty_setcolor(COLOR_ERROR);
        tty_printf("Извините, но это устройство(%x&%x) недоступно и будет проигнорировано.\n",VendorID,DeviceID);
        return;
    }
    tty_setcolor(COLOR_SYS_PATH);
    tty_printf("Устройство [%d&%d] было зарегистрировано с индексом %d\n",VendorID,DeviceID,i);
    tty_printf("Для получении информации об устройстве:\n");
    tty_printf("\t devmgr info %d\n",i);
}

/**
 * @brief DevMgr - Вывод информации об устройстве
 * @warning Функция сделана для консоли
 *
 * @param uint32_t id - Индекс устройства
 */
void devmgr_cli_info(uint32_t id){
    uint32_t i = id;
    tty_printf("[%d] %s\n",i,getDeviceName(i));
    tty_printf(" |--- Поставщик: [%x] %s\n",getDeviceInfo(i,DEVMGR_KEY_VENDORID),getVendorName(getDeviceInfo(i,DEVMGR_KEY_VENDORID)));
    tty_printf(" |--- Индентификатор устройства: %x\n",getDeviceInfo(i,DEVMGR_KEY_DEVICEID));
    tty_printf(" |--- Категория:\n");
    tty_printf(" | |--- [%d] %s\n",getDeviceInfo(i,DEVMGR_KEY_CLASS),getCategoryDevice(i,DEVMGR_KEY_CLASS));
    tty_printf(" |   |--- [%d] %s\n",getDeviceInfo(i,DEVMGR_KEY_SUBCLASS),getCategoryDevice(i,DEVMGR_KEY_SUBCLASS));
    tty_printf(" |     |--- [%d] %s\n",getDeviceInfo(i,DEVMGR_KEY_PROGIF),getCategoryDevice(i,DEVMGR_KEY_PROGIF));
    tty_printf(" |--- Состояние: %x\n",getDeviceInfo(i,DEVMGR_KEY_STATE));
    tty_printf(" |--- Индекс категории: %x\n",getDeviceInfo(i,DEVMGR_KEY_CATEGORY));
    tty_printf("\n");
}

/**
 * @brief DevMgr - Вывод информации об провайдере
 * @warning Функция сделана для консоли
 *
 * @param uint32_t id - Индентификатор провайдера
 */
void devmgr_cli_vendor(uint32_t id){
    uint32_t i = id;
    tty_printf("Поставщик: [%x] %s\n",id,getVendorName(id));
    tty_printf("\n");
}

/**
 * @brief Функция делает обращание к DevMgr
 * @warning Функция сделана для консоли
 *
 * @param uint32_t c - Кол-во аргументов
 * @param char* v[] - Аргементы
 *
 * @return uint32_t - Результат работы
 */
void devmgr_cli(uint32_t c,char* v[]){
    char* cmd = kheap_malloc(sizeof(char)*256);
    cmd = v[1];
    uint32_t param1 = atoi(v[2]);
    uint32_t param2 = atoi(v[3]);
    qemu_log("[Test KB] %d %s %d %d",c,cmd,param1,param2);
    tty_printf("Менеджер устройств v%d.%d.%d\n",DEVMGR_VERSION_MAJOR, DEVMGR_VERSION_MINOR, DEVMGR_VERSION_PATCH);
    if (strcmpn(cmd,"help") || c == 0){
        devmgr_cli_help();
        return;
    }
    if (strcmpn(cmd,"reg")){
        if (c != 3){
            tty_setcolor(COLOR_ERROR);
            tty_printf("Необходимо использовать 4 параметра!\n");
            tty_printf("Пример:\n");
            tty_printf("\t devmgr reg 32902 10200 - Подключит к системе уст-во с поставщиком 0x8086 и устройством 0x27D8\n");
            tty_printf("\t devmgr reg 4660 4369   - Подключит к системе уст-во с поставщиком 0x1234 и устройством 0x1111\n");
            tty_printf("\t devmgr reg 4660 4370   - Подключит к системе уст-во с поставщиком 0x1234 и устройством 0x1112\n");
            return;
        }
        devmgr_cli_reg(param1,param2);
        return;
    }
    if (strcmpn(cmd,"info")){
        if (c != 2){
            tty_setcolor(COLOR_ERROR);
            tty_printf("Необходимо использовать 3 параметра!\n");
            tty_printf("Пример:\n");
            tty_printf("\t devmgr info 0\n");
            return;
        }
        devmgr_cli_info(param1);
        return;
    }
    if (strcmpn(cmd,"vendor")){
        if (c != 2){
            tty_setcolor(COLOR_ERROR);
            tty_printf("Необходимо использовать 3 параметра!\n");
            tty_printf("Пример:\n");
            tty_printf("\t devmgr vendor 32902\n");
            return;
        }
        devmgr_cli_vendor(param1);
        return;
    }
    devmgr_cli_help();
    tty_setcolor(COLOR_ERROR);
    tty_printf("Неправильно заданы параметры.\n");
    tty_printf("\t devmgr '%s' '%d' '%d' '%d'",cmd,param1,param2,(strcmp(cmd,"reg")));
    return;
}
