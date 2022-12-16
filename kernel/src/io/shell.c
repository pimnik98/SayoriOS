/**
 * @file drv/input/keyboard.c
 * @author Пиминов Никита (nikita.piminoff@yandex.ru)
 * @brief Sayori Command Line (SCL -> Shell)
 * @version 0.3.0
 * @date 2022-10-20
 * @copyright Copyright SayoriOS Team (c) 2022
 */
#include <kernel.h>
#include <io/ports.h>
#include <io/colors.h>
#include <lib/stdio.h>
char* cmd = "";             ///< Текущая команда

/**
 * @brief Функция выводит экран справки
 *
 * @warning Функция сделана для консоли
 *
 * @param uint32_t c - Кол-во аргументов
 * @param char* v[] - Аргементы
 *
 * @return uint32_t - Результат работы
 */
uint32_t cmd_help(uint32_t c,char* v[]){
    tty_printf("Команды:\n" \
        "\t->./<file>            | Запустить программу в текущей директории\n" \
        "\t->cat   <filename>    | Выводит на экран содержимое файла\n" \
        "\t->cd    <folder>      | Переход в директорию\n" \
        "\t->cls                 | Выполняет отчистку экрана\n" \
        "\t->cpuinfo             | Информация о процессоре\n" \
        "\t->crash               | Вызывает BSOD\n" \
        "\t->font                | Рисует все доступные символы для шрифта\n" \
        "\t->help                | Открывает это окно справки\n" \
        "\t->ls                  | Отобразить список файлов (при указании второго параметра будет вывод файлов с той папки)\n" \
        "\t->shutdown            | Выключение устройства\n" \
        "\t->sysinfo             | Информация о системе\n" \
        "\t->reboot              | Перезагрузка устройства\n" \
        "\t->devmgr              | [ОТКЛ] Менеджер устройств\n" \
        "\t->pcilist             | [ОТКЛ] Отобразить список PCI-устройств\n" \
        "\t->view   <filename>   | [ОТКЛ] Отобразить картинку (для форматов Duke)\n" \
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
 *
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
 *
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
 *
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
 * @brief Вывод информации о системе
 */
void sysinfo(){
    tty_printf("Системная информация:\n");
    tty_printf("\tOS:               SayoriOS v%d.%d.%d\n",VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);
    tty_printf("\tДата сборки:      %s\n",__TIMESTAMP__);
    tty_printf("\tАрхитектура:      %s\n",ARCH_TYPE);
    tty_printf("\tПроцессор:        %s\n",getNameBrand());
    tty_printf("\tОЗУ:              %d kb\n",getInstalledRam());
    tty_printf("\tВидеоадаптер:     %s\n","Basic video adapter (Unknown)");
    tty_printf("\tДисплей:          %s (%dx%d)\n","(???)",getWidthScreen(),getHeightScreen());
    tty_printf("\tТики:             %d\n",getTicks());
}

/**
 * @brief Функция выодит инфу
 *
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
 * @brief Функция выводящая содержимое файла
 *
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
 * @brief Функция позволяющая узнать и установить имя пк
 *
 * @warning Функция сделана для консоли
 *
 * @param uint32_t c - Кол-во аргументов
 * @param char* v[] - Аргементы
 *
 * @return uint32_t - Результат работы
 */
uint32_t cmd_hostname(uint32_t c,char* v[]){
    if (c == 0){
        tty_printf("%s\n",getHostname());
        return 0;
    } else {
        setHostname(v[1]);
        tty_printf("%s\n",getHostname());
        return 0;
    }
}

/**
 * @brief Функция позволяющая узнать имя пользователя ОС
 *
 * @warning Функция сделана для консоли
 *
 * @param uint32_t c - Кол-во аргументов
 * @param char* v[] - Аргементы
 *
 * @return uint32_t - Результат работы
 */
uint32_t cmd_whoami(uint32_t c,char* v[]){
    tty_printf("%s\n",getUserName());
    return 0;
}

/**
 * @brief Функция откладки
 *
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

/**
 * @brief Функция откладки шрифта
 *
 * @warning Функция сделана для консоли
 *
 * @param uint32_t c - Кол-во аргументов
 * @param char* v[] - Аргементы
 *
 * @return uint32_t - Результат работы
 */
uint32_t cmd_font(uint32_t c,char* v[]){
    drawRect(0,0,800,200, 0);
    setColorFont(0xFFFFF);
    drawStringFont("АБВГДЕЁЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯ",10,30,0);
    setColorFont(0xCAFE12);
    drawStringFont("абвгдеёжзийклмнопрстуфхцчшщъыьэюя",10,60,0);
    setColorFont(0x333333);
    drawStringFont("!«№;%:?*()_+-=@#$^&[]{}|\\/ <>",10,90,0);
    setColorFont(0xDDDDDD);
    drawStringFont("QWERTYUIOPASDFGHJKLZXCVBNM",10,120,0);
    setColorFont(0xAAAAAA);
    drawStringFont("qwertyuiopasdfghjklzxcvbnm",10,150,0);
    setColorFont(0xFFFFFF);
    drawStringFont("1234567890.,",10,180,0);
}

/**
 * @brief Функция перехода в другую папку
 *
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
        tty_printf("[CMD] [CD] Необходимо указать путь для смены дириктории.\n");
        return 0;
    }
    qemu_log("Finding directory...");
    uint32_t inxDir = vfs_findDir(v[1]);
    qemu_log("Found directory!!!");
    if (inxDir == -1){
        tty_setcolor(COLOR_ERROR);
        tty_printf("[CMD] [CD] Папка `%s` не найдена.\n",v[1]);
        return 0;
    }
    setSysPath(v[1]);
    qemu_log("Now system path is: %s", v[1]);
    return 1;
}

/**
 * @brief Функция просмотра содержимого файла
 *
 * @warning Функция сделана для консоли
 *
 * @param uint32_t c - Кол-во аргументов
 * @param char* v[] - Аргементы
 *
 * @return uint32_t - Результат работы
 */
uint32_t cmd_ls(uint32_t c,char* v[]){
    char path[256] = {0};
	if(c==0) {
    	qemu_log("Listing local folder: %s", getSysPath());
	}else{
    	qemu_log("Listing folder: %s", v[1]);		
	}
    strcpy(path, (c == 0?getSysPath():v[1]));
    if (strlen(path) <= 0){
        tty_setcolor(COLOR_ERROR);
        tty_printf("[CMD] [LS] Некорректные данные `%s`.\n",path);
        return 0;
    }
    uint32_t inxDir = vfs_findDir(path);
    if (inxDir == -1){
        tty_setcolor(COLOR_ERROR);
        tty_printf("[CMD] [LS] Папка `%s` не найдена.\n",v[1]);
        return 0;
    }
    size_t sss = vfs_getCountElemDir(path);
    struct dirent* testFS = vfs_getListFolder(path);
    for(size_t f = 0;sss > f;f++){
        tty_printf("[%s] %s (%d б)\n",(testFS[f].type == FS_FILE?"Файл":"Папка"),testFS[f].name,testFS[f].length);
    }
    return 1;
}

/**
 * @brief Функция для отключенных фнк
 *
 * @warning Функция сделана для консоли
 *
 * @param uint32_t c - Кол-во аргументов
 * @param char* v[] - Аргементы
 *
 * @return uint32_t - Результат работы
 */
uint32_t cmdDisabled(uint32_t c,char* v[]){
    tty_setcolor(COLOR_ERROR);
    tty_printf("[CMD] [ОШИБКА] Команда `%s` отключена или не реализована.\n",v[0]);
    return -1;
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
    strcpy(temp, getSysPath());
    strcat(temp, v[1]);
    FILE* elf_exec = fopen(temp,"r");
    if (ferror(elf_exec) != 0){
        tty_setcolor(COLOR_ERROR);
        tty_printf("[CMD] [EXEC] Программа `%s` не найдена в текущей папке.\n",v[1]);
        return 2;
    }
    //elf_info(temp);

    run_elf_file(temp, c, v);
    return 0;
}

/**
 * @brief Функция обработки комманд в Shell
 *
 * @warning Функция сделана для консоли
 *
 * @param char* ncmd - Команда
 *
 * @return uint32_t - Результат работы
 */
void cmdHandler(char* ncmd){
        uint32_t argc = str_cdsp(ncmd," ");
        char* argv[128] = {0};
        str_split(ncmd,argv," ");
        cmd = ncmd;
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
            //cmdDisabled(argc,argv);
            cmd_exec(argc+1,argv);
            return;
        } else if (strcmpn(argv[0],"view")){
            cmdDisabled(argc,argv);
            //cmd_view(argc,argv);
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
            cmdDisabled(argc,argv);
            //cmd_pcilist(argc,argv);
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
        } else if (strcmpn(argv[0],"crash")){
            int a = 0/0;
            return;
        } else if (strcmpn(argv[0],"devmgr")){
            cmdDisabled(argc,argv);
            //devmgr_cli(argc,argv);
            return;
        } else {
            //tty_setcolor(COLOR_ERROR);
            //tty_printf("[CMD] Команда `%s` не найдена. Введите \"help\" для получения списка команд.\n",argv[0]);
            //return;
            char* run[2] = {0};
            run[0] = "run";
            run[1] = argv[0];
            cmd_exec(2,run);
            return;
        }
}

/**
 * @brief Точка входа в консоль
 */
void shell(){
    //changeStageKeyboard(1);
    tty_set_bgcolor(COLOR_BG);
    tty_setcolor(COLOR_ALERT);
    tty_printf("\nВведите \"help\" для получения списка команд.\n");
    char* ncmd = kmalloc(sizeof(char) * 256);
    tty_setcolor(COLOR_TEXT);
    while (1) {
        tty_setcolor(COLOR_SYS_TEXT);
        tty_printf("\n%s@%s:",getUserName(),getHostname());
        tty_setcolor(COLOR_SYS_PATH);
        tty_printf("%s>", getSysPath());

        tty_setcolor(COLOR_TEXT);

        memset(ncmd, 0, 256);
        // strcpy(ncmd,getStringBufferKeyboard());
        gets(ncmd);
        if (strlen(ncmd) > 256) {
            tty_setcolor(COLOR_ERROR);
            tty_printf("\nERROR: limit 256 char's!");
            continue;
        }
        qemu_log("[CMD] '%s'",ncmd);
        cmdHandler(ncmd);
    }
}

