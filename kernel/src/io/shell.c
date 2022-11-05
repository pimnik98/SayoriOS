/**
 * @file kernel.c
 * @author Никита Пиминов (github.com/pimnik98)
 * @brief Входная точка консоли, Sayori Command Line Interface
 * @version 0.0.5
 * @date 2022-09-18
 * @copyright Copyright SayoriOS
 */
#include <kernel.h>
#include <drivers/devmgr.h>
#include <libk/string.h>
#include <io/imaging.h>

char current_dir[256] = "/initrd/apps/";
char* whoami = "root";
char* pcname = "oem";
char* cmd = "";

/**
 * @brief Функция выводит экран справки
 * @warning Функция сделана для консоли
 *
 * @param uint32_t c - Кол-во аргументов
 * @param char* v[] - Аргементы
 *
 * @return uint32_t - Результат работы
 */
uint32_t cmd_help(uint32_t c,char* v[]){
    tty_printf("Команды:\n" \
        "\t->help                | открывает это окно справки\n" \
        "\t->cls                 | выполняет отчистку экрана\n" \
        "\t->cat   <filename>    | выводит на экран содержимое файла\n" \
        "\t->cd    <folder>      | переход в директорию\n" \
        "\t->./<file>            | запустить программу в текущей директории\n" \
        "\t->ls                  | отобразить список файлов\n" \
        "\t->sysinfo             | информация о системе\n" \
        "\t->pcilist             | Отобразить список PCI-устройств\n" \
        "\t->cpuinfo             | Информация о процессоре\n" \
        "\t->reboot              | Перезагрузка устройства\n" \
        "\t->shutdown            | Выключение устройства\n" \
        "\t->view   <filename>   | Отобразить картинку (для форматов Duke)\n" \
        "\t->font                | Рисует все доступные символы для шрифта\n" \
        "\t->devmgr              | Менеджер устройств\n" \
        "\n"
    );
    return 0;
}

uint32_t cmd_cpuinfo(uint32_t c,char* v[]){
    detect_cpu(0);
    return 0;
}
/**
 * @brief Функция позволяет отчистить экран
 * @warning Функция сделана для консоли
 *
 * @param uint32_t c - Кол-во аргументов
 * @param char* v[] - Аргементы
 *
 * @return uint32_t - Результат работы
 */
uint32_t cmd_cls(uint32_t c,char* v[]){
    clean_screen();
    return 0;
}

/**
 * @brief Функция перезагружающая устройство
 * @warning Функция сделана для консоли
 *
 * @param uint32_t c - Кол-во аргументов
 * @param char* v[] - Аргементы
 *
 * @return uint32_t - Результат работы
 */
uint32_t cmd_reboot(uint32_t c,char* v[]){
    tty_printf("REBOOT NOW!\n");
    sleep_ms(100);
    reboot();
    return 0;
}
/**
 * @brief Функция выключающая устройство
 * @warning Функция сделана для консоли
 *
 * @param uint32_t c - Кол-во аргументов
 * @param char* v[] - Аргементы
 *
 * @return uint32_t - Результат работы
 */
uint32_t cmd_shutdown(uint32_t c,char* v[]){
    tty_printf("SHUTDOWN NOW!\n");
    sleep_ms(100);
    shutdown();
    return 0;
}
/**
 * @brief Функция выводит список PCI-устройств
 * @warning Функция сделана для консоли
 *
 * @param uint32_t c - Кол-во аргументов
 * @param char* v[] - Аргементы
 *
 * @return uint32_t - Результат работы
 */
uint32_t cmd_pcilist(uint32_t c,char* v[]){
    tty_printf("Найденные PCI-устройства:\n");
    for(uint32_t i = 0;i<getCountDevices();i++){
        tty_printf("[%d] %s\n",i,getDeviceName(i));
        tty_printf(" |--- Поставщик: [%x] %s\n",getDeviceInfo(i,DEVMGR_KEY_VENDORID),getVendorName(getDeviceInfo(i,DEVMGR_KEY_VENDORID)));
        tty_printf(" |--- DeviceID: %x\n",getDeviceInfo(i,DEVMGR_KEY_DEVICEID));
        tty_printf(" |--- Категория:\n");
        tty_printf(" | |--- [%d] %s\n",getDeviceInfo(i,DEVMGR_KEY_CLASS),getCategoryDevice(i,DEVMGR_KEY_CLASS));
        tty_printf(" |   |--- [%d] %s\n",getDeviceInfo(i,DEVMGR_KEY_SUBCLASS),getCategoryDevice(i,DEVMGR_KEY_SUBCLASS));
        tty_printf(" |     |--- [%d] %s\n",getDeviceInfo(i,DEVMGR_KEY_PROGIF),getCategoryDevice(i,DEVMGR_KEY_PROGIF));
        tty_printf(" |--- Статус: %x\n",getDeviceInfo(i,DEVMGR_KEY_STATE));
        tty_printf(" |--- CategoryID: %x\n",getDeviceInfo(i,DEVMGR_KEY_CATEGORY));
        tty_printf("\n");
    }
    //checkAllBuses();
    return 0;
}
/**
 * @brief Функция выодит инфу
 * @warning Функция сделана для консоли
 *
 * @param uint32_t c - Кол-во аргументов
 * @param char* v[] - Аргементы
 *
 * @return uint32_t - Результат работы
 */
uint32_t cmd_sysinfo(uint32_t c,char* v[]){
    sysinfo();
    return 0;
}
/**
 * @brief Функция выводящая список файлов
 * @warning Функция сделана для консоли
 *
 * @param uint32_t c - Кол-во аргументов
 * @param char* v[] - Аргементы
 *
 * @return uint32_t - Результат работы
 */
uint32_t cmd_ls(uint32_t c,char* v[]){
    initrd_list(0, 0);
    return 0;
}
/**
 * @brief Функция выводящая содержимое файла
 * @warning Функция сделана для консоли
 *
 * @param uint32_t c - Кол-во аргументов
 * @param char* v[] - Аргементы
 *
 * @return uint32_t - Результат работы
 */
uint32_t cmd_cat(uint32_t c,char* v[]){
    if (c == 0){
        tty_setcolor(COLOR_ERROR);
        tty_printf("[CMD] [CAT] Необходимо указать путь для чтения файла.\n");
        return 1;
    }
    FILE* cat_file = fopen(v[1],"r");
    if (ferror(cat_file) != 0){
        tty_setcolor(COLOR_ERROR);
        tty_printf("[CMD] [CAT] Не удалось найти файл `%s`. Проверьте правильность введенного вами пути.\n",v[1]);
        return 2;
    }
    char * buffer = fread(cat_file);
    fclose(cat_file);
    tty_printf("%s\n",buffer);
    return 0;
}
/**
 * @brief Функция позволяющая установить текущию папку
 * @warning Функция сделана для консоли
 *
 * @param uint32_t c - Кол-во аргументов
 * @param char* v[] - Аргементы
 *
 * @return uint32_t - Результат работы
 */
uint32_t cmd_cd(uint32_t c,char* v[]){
    if (c == 0){
        tty_setcolor(COLOR_ERROR);
        tty_printf("[CMD] [CD] Вы должны указать каталог, к которому вы хотите изменить путь.\n");
        return 1;
    }
    char* dname = v[1];
    if (dname[0] != '/') {
        char temp[256];
        strcpy(temp, current_dir);
        temp[strlen(temp) - 1] = 0;
        strcat(temp, dname);
        temp[strlen(temp) - 1] = 0;
        temp[strlen(temp) - 1] = 0;
        strcpy(dname, temp);
    }
    if (dname[strlen(dname) - 1] != '/') {
        strcat(dname, "/");
    }
    if (vfs_exists(dname) && vfs_is_dir(dname)) {
        strcpy(current_dir, dname);
    } else {
        tty_setcolor(COLOR_ERROR);
        tty_printf("[CMD] [CD] Извините, но папка `%s` не найдена. Проверьте правильность введенного вами пути.\n",v[1]);
        return 1;
    }
    return 0;
}
/**
 * @brief Функция позволяющая узнать и установить имя пк
 * @warning Функция сделана для консоли
 *
 * @param uint32_t c - Кол-во аргументов
 * @param char* v[] - Аргементы
 *
 * @return uint32_t - Результат работы
 */
uint32_t cmd_hostname(uint32_t c,char* v[]){
    if (c == 0){
        tty_printf("%s\n",pcname);
        return 0;
    } else {
        pcname = v[1];
        tty_printf("%s\n",pcname);
        return 0;
    }
}

/**
 * @brief Функция позволяющая узнать имя пользователя ОС
 * @warning Функция сделана для консоли
 *
 * @param uint32_t c - Кол-во аргументов
 * @param char* v[] - Аргементы
 *
 * @return uint32_t - Результат работы
 */
uint32_t cmd_whoami(uint32_t c,char* v[]){
    tty_printf("%s\n",whoami);
    return 0;
}

/**
 * @brief Функция просмотра изображения
 * @warning Функция сделана для консоли
 *
 * @param uint32_t c - Кол-во аргументов
 * @param char* v[] - Аргементы
 *
 * @return uint32_t - Результат работы
 */
uint32_t cmd_view(uint32_t c,char* v[]){
    if (c == 0){
        tty_setcolor(COLOR_ERROR);
        tty_printf("[CMD] [VIEW] Необходимо указать путь для отображения изображения.\n");
        return 1;
    }
    FILE* c_view = fopen(v[1],"r");
    if (ferror(c_view) != 0){
        tty_setcolor(COLOR_ERROR);
        tty_printf("[CMD] [VIEW] Не удалось найти файл `%s`. Проверьте правильность введенного вами пути.\n",v[1]);
        return 2;
    }
    struct DukeImageMeta* data = duke_get_image_metadata(v[1]);
    if(data != 0) {
        duke_draw_from_file(v[1], getWidthScreen() - data->width - 8, 0);
        return 0;
    }else{
        tty_setcolor(COLOR_ERROR);
        tty_printf("[CMD] [VIEW] Возникли проблемы с открытием файла `%s`. Может быть этот файл не в формате Duke.\n",v[1]);
        return 3;
    }
    return 0;
}



/**
 * @brief Функция выполнения программы
 * @warning Функция сделана для консоли
 *
 * @param uint32_t c - Кол-во аргументов
 * @param char* v[] - Аргементы
 *
 * @return uint32_t - Результат работы
 */
uint32_t cmd_exec(uint32_t c,char* v[]){
    if (c == 0){
        tty_setcolor(COLOR_ERROR);
        tty_printf("[CMD] [EXEC] Для выполнения этой команды необходимо указать имя запускаемого файла.\n");
        return 1;
    }
    char temp[256] = {0};
    strcpy(temp, current_dir);
    strcat(temp, v[1]);
    FILE* elf_exec = fopen(temp,"r");
    if (ferror(elf_exec) != 0){
        tty_setcolor(COLOR_ERROR);
        tty_printf("[CMD] [EXEC] Программа `%s` не найдена в текущей папке.\n",v[1]);
        return 2;
    }
    //elf_info(temp);
    run_elf_file(temp, 0, 0);
    return 0;
}

/**
 * @brief Функция откладки
 * @warning Функция сделана для консоли
 *
 * @param uint32_t c - Кол-во аргументов
 * @param char* v[] - Аргементы
 *
 * @return uint32_t - Результат работы
 */
uint32_t cmd_debug(uint32_t c,char* v[]){
    for(int i = 0; c >= i; i++){
        qemu_log("[CMD] argc: %d => argv: %s",i,v[i]);
    }
}

uint32_t cmd_font(uint32_t c,char* v[]){
    drawRect(0,0,800,200);
    setColorFont(0xFFFFF);
    drawStringFont("АБВГДЕЁЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯ",10,30,0);
    setColorFont(0xCAFE12);
    drawStringFont("абвгдеёжзийклмнопрстуфхцчшщъыьэюя",10,60,0);
    setColorFont(0x333333);
    drawStringFont("!«№;%:?*()_+-=@#$^&[]{}|\\/",10,90,0);
    setColorFont(0xDDDDDD);
    drawStringFont("QWERTYUIOPASDFGHJKLZXCVBNM",10,120,0);
    setColorFont(0xAAAAAA);
    drawStringFont("qwertyuiopasdfghjklzxcvbnm",10,150,0);
    setColorFont(0xFFFFFF);
    drawStringFont("1234567890.,",10,180,0);
}

void cmdHandler(char* ncmd){
        uint32_t argc = str_cdsp(ncmd," ");
        char* argv[128] = {0};
        str_split(ncmd,argv," ");
        cmd = ncmd;
        tty_printf("\n");
        for(int i = 0; argc >= i; i++){
            qemu_log("[CMD] '%s' => argc: %d => argv: %s",ncmd,i,argv[i]);
        }

        if (strcmpn(argv[0],"help")){
            cmd_help(argc,argv);
            return;
        } else if (strcmpn(argv[0],"debug")){
            tty_printf("debug\n");
            cmd_debug(argc,argv);
            return;
        } else if (strcmpn(argv[0],"run")){
            cmd_exec(argc,argv);
            return;
        } else if (strcmpn(argv[0],"view")){
            cmd_view(argc,argv);
            return;
        } else if (strcmpn(argv[0],"cd")){
            cmd_cd(argc,argv);
            return;
        } else if (strcmpn(argv[0],"cat")){
            cmd_cat(argc,argv);
            return;
        } else if (strcmpn(argv[0],"cls")){
            cmd_cls(argc,argv);
            return;
        } else if (strcmpn(argv[0],"reboot")){
            cmd_reboot(argc,argv);
            return;
        } else if (strcmpn(argv[0],"shutdown")){
            cmd_shutdown(argc,argv);
            return;
        } else if (strcmpn(argv[0],"pcilist")){
            cmd_pcilist(argc,argv);
            return;
        } else if (strcmpn(argv[0],"sysinfo")){
            cmd_sysinfo(argc,argv);
            return;
        } else if (strcmpn(argv[0],"ls")){
            cmd_ls(argc,argv);
            return;
        } else if (strcmpn(argv[0],"cpuinfo")){
            cmd_cpuinfo(argc,argv);
            return;
        } else if (strcmpn(argv[0],"whoami")){
            cmd_whoami(argc,argv);
            return;
        } else if (strcmpn(argv[0],"hostname")){
            cmd_hostname(argc,argv);
            return;
        } else if (strcmpn(argv[0],"font")){
            cmd_font(argc,argv);
            return;
        } else if (strcmpn(argv[0],"devmgr")){
            devmgr_cli(argc,argv);
            return;
        } else {
            char* run[2] = {0};
            run[1] = argv[0];
            cmd_exec(1,run);
            return;
        }
}


/**
 * @brief Входная точка консоли
 *
 */
void shell() {
    changeStageKeyboard(1);
    tty_setcolor(COLOR_ALERT);
    tty_printf("\nВведите \"help\" для получения списка команд.\n");

    tty_setcolor(COLOR_TEXT);

    while (1) {
        tty_setcolor(COLOR_SYS_TEXT);
        tty_printf("\n%s@%s:",whoami,pcname);
        tty_setcolor(COLOR_SYS_PATH);
        tty_printf("%s>", current_dir);

        tty_setcolor(COLOR_TEXT);

        char* ncmd = keyboard_gets();

        if (strlen(ncmd) == 0) {
            continue;
        } else if (strlen(ncmd) > 256) {
            tty_setcolor(COLOR_ERROR);
            tty_printf("\nERROR: limit 256 char's!");
            continue;
        }
        qemu_log("[CMD] '%s'",ncmd);
        cmdHandler(ncmd);
    }
}


/**
 * @brief Вывод информации о системе
 * 
 */
void sysinfo(){
    drawASCIILogo(1);
    tty_printf("Системная информация:\n");
    tty_printf("\tOS:               SayoriOS v%d.%d.%d\n",VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);
    tty_printf("\tДата сборки:      %s\n",__TIMESTAMP__);
    tty_printf("\tАрхитектура:      %s\n",ARCH_TYPE);
    tty_printf("\tПроцессор:        %s\n",getNameBrand());
    tty_printf("\tОЗУ:              %d kb\n",getInstalledRam());
    tty_printf("\tВидеоадаптер:     %s\n","Basic video adapter (Unknown)");
    tty_printf("\tДисплей:          %s (%dx%d)\n","(???)",getWidthScreen(),getHeightScreen());
    tty_printf("\tТики:             %d\n",timer_get_ticks());
}


