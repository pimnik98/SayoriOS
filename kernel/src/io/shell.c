/**
 * @file drv/input/keyboard.c
 * @author Пиминов Никита (nikita.piminoff@yandex.ru), Drew >_ (pikachu_andrey@vk.com)
 * @brief Sayori Command Line (SCL -> Shell)
 * @version 0.3.2
 * @date 2022-10-20
 * @copyright Copyright SayoriOS Team (c) 2022-2023
 */
#include <kernel.h>
#include <io/ports.h>
#include <io/imaging.h>
#include <io/colors.h>
#include <lib/stdio.h>
#include <fs/milla.h>
#include <gui/parallel_desktop.h>

typedef struct {
	char* name;
	uint32_t (*funcv)(uint32_t, char**);
	char* helpstring;
} Command_t;

char* cmd = "";             ///< Текущая команда

Command_t shell_commands[];

void piano();

uint32_t cmd_crash() {
    size_t a = 2/0;
    detect_cpu(a);
    return 0;
}

uint32_t cmd_help(uint32_t c,char* v[]){
    size_t hlp_padding = 24;
    qemu_log("Доступные команды:");
    for(size_t i = 0; shell_commands[i].name != NULL; i++) {
        _tty_printf("\t%s", shell_commands[i].name);
        for(size_t j = 0; j < hlp_padding - strlen(shell_commands[i].name); j++) {
            _tty_printf(" ");
        }
        tty_printf(" | %s\n", shell_commands[i].helpstring);
    }
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
    clean_tty_screen();

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
    tty_printf("\tДата сборки:      %s\n", __TIMESTAMP__);
    tty_printf("\tАрхитектура:      %s\n", ARCH_TYPE);
    tty_printf("\tПроцессор:        %s\n", getNameBrand());
    tty_printf("\tОЗУ:              %d kb\n",getInstalledRam()/1024);
    tty_printf("\tВидеоадаптер:     %s\n","Basic video adapter (Unknown)");
    tty_printf("\tДисплей:          %s (%dx%d)\n","(\?\?\?)",getWidthScreen(),getHeightScreen());
    tty_printf("\tТики:             %d\n",getTicks());
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
 * @brief Функция выводящая содержимое файла
 *
 * @warning Функция сделана для консоли
 *
 * @param uint32_t c - Кол-во аргументов
 * @param char* v[] - Аргементы
 *
 * @return uint32_t - Результат работы
 */
uint32_t cmd_cat(uint32_t c, char* v[]){
    if (c == 0){
        tty_setcolor(COLOR_ERROR);
        tty_printf("[CMD] [CAT] Необходимо указать путь для чтения файла.\n");
        return 1;
    }

	char* file = v[1];
	if(file[0] == '&') {
        if(strcmpn(file + 1, "com1")) {
            tty_printf("Внимание: Этот порт предназначен для отладки ядра.\n");
            for(;;) {
                tty_printf("%c", serial_readchar(PORT_COM1));
            }
        }
        
        return 0;
	}

    char* full_path = kcalloc(strlen(getSysPath()) + strlen(v[1]) + 1, 1);
	
    strcpy(full_path, getSysPath());
    strcat(full_path, v[1]);

    FILE* cat_file = fopen(full_path, "r");
    if (!cat_file){
        tty_setcolor(COLOR_ERROR);
        tty_printf("[CMD] [CAT] Не удалось найти файл `%s`. Проверьте правильность введенного вами пути.\n",v[1]);
        
        kfree(full_path);
        return 2;
    }

    size_t filesize = fsize(cat_file);
    
    char* buffer = kcalloc(filesize + 1, 1);

    fread_c(cat_file, filesize, 1, buffer);

    tty_puts(buffer);
    
    kfree(full_path);
    kfree(buffer);
    
    fclose(cat_file);

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
// uint32_t cmd_ac97(uint32_t c, char* v[]){
//     if (c == 0){
//         tty_setcolor(COLOR_ERROR);
//         tty_printf("[CMD] [AC97] Необходимо указать путь для открытия файла.\n");
//         return 1;
//     }

//     FILE* file = fopen(v[1],"rb");
//     tty_printf("[CMD] [AC97] Попытка открытия файла %s...\n",v[1]);

//     if (ferror(file) != 0){
//         tty_setcolor(COLOR_ERROR);
//         tty_printf("[CMD] [AC97] Не удалось найти файл `%s`. Проверьте правильность введенного вами пути.\n",v[1]);
//         return 2;
//     }

//     fclose(file);
//     return 0;
// }

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
    if (c > 0){
        setUserName(v[1]);
    }
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
    return 0;
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
    return 0;
}

uint32_t cmd_milla(uint32_t c,char* v[]){
    if (c == 0){
        tty_printf("тут типа инструкция :)");
        return 0;
    } else if (c < 2){
        tty_setcolor(COLOR_ERROR);
        tty_printf("[CMD] [Milla] Минимум 3 аргумента. Код ошибки: %d\n");
        return 0;
    }
    int init = __milla_init();
    if (init != 0){
        tty_setcolor(COLOR_ERROR);
        tty_printf("[CMD] [Milla] Не удалось запустить Milla. Код ошибки: %d\n",init);
        return -1;
    }
    /*init = __milla_cleanState();
    if (init != 1){
        tty_setcolor(COLOR_ERROR);
        tty_printf("[CMD] [Milla] Не удалось отчистить буфер Milla. Код ошибки: %d\n",init);
        return -2;
    }*/

    if (strcmpn(v[1], "list")) {
        char* listt = __milla_getList(v[2]);
        //tty_printf("\n%s\n",listt);

        uint32_t _m_d1 = str_cdsp(listt,"\n");
        char* _m_d[256] = {0};
        str_split(listt,_m_d,"\n");
        uint32_t _m_d2 = 0;

        char* _m_s[256] = {0};
        tty_printf("Найдено файлов и папок: %d\n",_m_d1);
        for(size_t ind = 0; ind < _m_d1; ind++){
            _m_d2 = str_cdsp(_m_d[ind],"::");
            if (_m_d2 < 1) continue;
            memset(_m_s,0,256);
            str_split(_m_d[ind],_m_s,"::");
            tty_printf("|--- %s\n",_m_s[1]);
            tty_printf("| |--- Тип: %s\n",_m_s[0]);
            tty_printf("| |--- Формат: %s\n",_m_s[2]);
            tty_printf("| |--- Размер: %s\n",_m_s[4]);
            tty_printf("| |--- Дата: %s\n",_m_s[3]);
            tty_printf("| |--- Владелец: %s\n",_m_s[5]);
            tty_printf("|\n");

        }


        kfree(listt);
        kfree((void*)_m_d);
        kfree((void*)_m_s);
        kfree((void*)_m_d1);
        kfree((void*)_m_d2);
        return 1;

    } else if (strcmpn(v[1],"info")){
        char* devName = kmalloc(sizeof(char)*256);
        memset(devName, 0, 256);
        strcpy(devName, (char*)__milla_getDiskInfo_Name());

        tty_printf("[Milla]\n * Состояние: Подключено\n * Имя диска: %s\n * Доступное пространство: %dkb.\n * Занятое пространство: %dkb.\n * Всего доступно: %dkb.\n", devName, __milla_getDiskInfo_Free()/1024, __milla_getDiskInfo_Use()/1024, __milla_getDiskInfo_All()/1024);
        kfree(devName);
    } else if (strcmpn(v[1], "mlist")) {
        char* listt = __milla_getList(v[2]);
        //tty_printf("\n%s\n",listt);

        uint32_t _m_d1 = str_cdsp(listt,"\n");
        char* _m_d = kmalloc(256);
        str_split(listt, _m_d, "\n");
        uint32_t _m_d2 = 0;

        char* _m_s = kmalloc(256);
        tty_printf("Найдено файлов и папок: %d\n",_m_d1);
        for(size_t ind = 0; ind < _m_d1; ind++){
            _m_d2 = str_cdsp(_m_d[ind],"::");
            if (_m_d2 < 1) continue;
            memset(_m_s, 0, 256);
            str_split(_m_d[ind],_m_s,"::");
            tty_printf("%s ",_m_s[1]);
        }


        kfree(listt);
        kfree((void*)_m_d);
        kfree((void*)_m_s);
        // kfree((void*)_m_d1);
        kfree((void*)_m_d2);
    } else if (strcmpn(v[1], "clist")) {
        char* listt = __milla_getList(v[2]);
        uint32_t _m_d1 = str_cdsp(listt,"\n");
        char* _m_d[256] = {0};
        str_split(listt,_m_d,"\n");
        uint32_t _m_d2 = 0;
        tty_printf("Найдено файлов и папок: %d\n",_m_d1);
        for(size_t ind = 0; ind < _m_d1; ind++){
            _m_d2 = str_cdsp(_m_d[ind],"::");
            if (_m_d2 < 1) continue;
            tty_printf("%s\n",_m_d[ind]);
        }
        kfree(listt);
        kfree((void*)_m_d);
        kfree((void*)_m_d1);
        kfree((void*)_m_d2);
        return 1;
    } else if (strcmpn(v[1], "file")) {
        char* file = __milla_getFile(v[2]);
        tty_printf("READ: %s\n",file);
        kfree(file);

        return 1;
    } else if (strcmpn(v[1], "write")) {
        if (c < 3){
            tty_setcolor(COLOR_ERROR);
            tty_printf("[CMD] [Milla] Минимум 4 аргумента. Код ошибки: %d\n",0);
            return 1;
        }
        init = __milla_writeFile(v[2],v[3]);
        if (init == -1){
            tty_setcolor(COLOR_ERROR);
            tty_printf("[CMD] [Milla] Не удалось отправить путь файла. Код ошибки: %d\n",init);
            return init;
        } else if (init == -2){
            tty_setcolor(COLOR_ERROR);
            tty_printf("[CMD] [Milla] Не удалось отправить размер файла. Код ошибки: %d\n",init);
            return init;
        } else if (init == -3){
            tty_setcolor(COLOR_ERROR);
            tty_printf("[CMD] [Milla] Конечное количество отправленных данных не совпадает. (%d != %d)\n",init, strlen(v[3]));
            return init;
        }
        tty_printf("[CMD] [Milla] Файл отправлен. (Вес :%d | Отправлено: %d)\n",init,strlen(v[3]));
        return init;
    } else if (strcmpn(v[1], "mkdir")) {
        init = __milla_mkdir(v[2]);
        if (init != 1){
            tty_setcolor(COLOR_ERROR);
            tty_printf("[CMD] [Milla] Не удалось создать папку. Код ошибки: %d\n",init);
            return init;
        }
        tty_printf("[CMD] [Milla] Папка %s успешно создана.",v[2]);
        return init;
    } else if (strcmpn(v[1], "touch")) {
        init = __milla_touch(v[2]);
        if (init != 1){
            tty_setcolor(COLOR_ERROR);
            tty_printf("[CMD] [Milla] Не удалось создать файл. Код ошибки: %d\n",init);
            return init;
        }
        tty_printf("[CMD] [Milla] Файл %s успешно создан.",v[2]);
        return init;
    } else if (strcmpn(v[1], "del")) {
        init = __milla_delete(v[2]);
        if (init != 1){
            tty_setcolor(COLOR_ERROR);
            tty_printf("[CMD] [Milla] Не удалось удалить файл. Код ошибки: %d\n",init);
            return init;
        }
        tty_printf("[CMD] [Milla] Файл %s успешно удален.",v[2]);
        return init;
    } else {
        tty_setcolor(COLOR_ERROR);
        tty_printf("[CMD] [Milla] Команда не поддерживается!\n");
        return 0;
    }
    return init;
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
uint32_t cmd_cd(uint32_t c, char* v[]){
    if (c == 0){
        tty_setcolor(COLOR_ERROR);
        tty_printf("[CMD] [CD] Необходимо указать путь для смены дириктории.\n");
        return 0;
    }

    char* path_to = v[1];

    size_t path_len = strlen(path_to) + strlen(getSysPath());
    char* new_path = kcalloc(path_len + 2, 1);

    // Is a absolute path?
    if(path_to[0] == '/') {
        qemu_log("First variant;");
        strcpy(new_path, path_to);
    } else {
        // Relative path.
        qemu_log("Second variant (syspath: %s, path: %s);", getSysPath(), path_to);
        strcpy(new_path, getSysPath());
        strcat(new_path, path_to);
    }

    // If we want to turn back.
    if(strcmpn(path_to, "..")) {
        size_t len = strlen(new_path);

        do {
            if(new_path[len] != '/')
                break;

            new_path[len] = 0;
        } while(len--);

        do {
            if(new_path[len] == '/')
                break;

            new_path[len] = 0;
        } while(len--);

        len--;

        do {
            if(new_path[len] == '/')
                break;

            new_path[len] = 0;
        } while(len--);
    }

    // Sayori's VFS is very strange, it's a fix.
    if(new_path[path_len] != '/') {
        new_path[path_len] = '/';
    }

    qemu_log("Total path: %s", new_path);
    
    uint32_t inxDir = vfs_findDir(new_path);
    
    if (inxDir == -1){
        tty_setcolor(COLOR_ERROR);
        tty_printf("[CMD] [CD] Папка `%s` не найдена.\n", new_path);

        kfree(new_path);
        return 0;
    }

    setSysPath(new_path);
    qemu_log("Now system path is: %s", getSysPath());

    kfree(new_path);
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
uint32_t cmd_ls(uint32_t c, char* v[]){
    char path[256] = {0};
	if(c==0) {
    	qemu_log("Listing local folder: %s", getSysPath());
	}else{
    	qemu_log("Listing folder: %s", v[1]);		
	}
    strcpy(path, (!c?getSysPath():v[1]));
    if (strlen(path) <= 0){
        tty_setcolor(COLOR_ERROR);
        tty_printf("[CMD] [LS] Некорректные данные `%s`. (strlen(path) <= 0)\n",path);
        return 0;
    }
    uint32_t inxDir = vfs_findDir(path);
    if (inxDir == -1){
        tty_setcolor(COLOR_ERROR);
        tty_printf("[CMD] [LS] Папка `%s` не найдена.\n",v[1]);
        return 0;
    }
    size_t sss = vfs_getCountElemDir(path);
    // qemu_log("SSS (?) is: %d (%x)", sss, sss);

    struct dirent* testFS = vfs_getListFolder(path);
    // qemu_log("testFS pointer is: %d (%x)", testFS, testFS);
    for(size_t f = 0; sss > f; f++){
        if(testFS[f].type == FS_FILE) {
            tty_printf("[%s] %s (%d kb)\n",(testFS[f].type == FS_FILE?"Файл":"Папка"), testFS[f].name, vfs_byteToKByte(testFS[f].length));
        }else{
            tty_printf("[%s] %s\n",(testFS[f].type == FS_FILE?"Файл":"Папка"), testFS[f].name);
        }
        // qemu_printf("[%s] %s (%d kb)\n",(testFS[f].type == FS_FILE?"Файл":"Папка"),testFS[f].name,vfs_byteToKByte(testFS[f].length));
    }

    kfree(testFS);

    vfs_unlistFolder(path, testFS);
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

uint32_t find_and_run_program(uint32_t c, char* v[]){
    if (c == 0){
        tty_setcolor(COLOR_ERROR);
        tty_printf("[CMD] [EXEC] Для выполнения этой команды необходимо указать имя запускаемого файла.\n");
        return 1;
    }

    char* temp = kcalloc(strlen(getSysPath()) + strlen(v[0]) + 1, 1);

    strcpy(temp, getSysPath());
    strcat(temp, v[0]);
    
    FILE* elf_exec = fopen(temp, "r");

    if(!elf_exec){
        tty_setcolor(COLOR_ERROR);
        tty_printf("[CMD] [EXEC] Программа `%s` не найдена в текущей папке.\n", temp);
        kfree(temp);
        return 2;
    }

    if(!is_elf_file(elf_exec)) {
        tty_setcolor(COLOR_ERROR);
        tty_printf("[CMD] [EXEC] Файл `%s` не является программой.\n", temp);
        kfree(temp);
        return 2;
    }

    run_elf_file(temp, c, v);
    
    fclose(elf_exec);

    kfree(temp);

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
uint32_t cmd_exec(uint32_t c, char* v[]){
    if (c == 0){
        tty_setcolor(COLOR_ERROR);
        tty_printf("[CMD] [EXEC] Для выполнения этой команды необходимо указать имя запускаемого файла.\n");
        return 1;
    }
    char temp[256] = {0};
    strcpy(temp, getSysPath());
    strcat(temp, v[1]);
    FILE* elf_exec = fopen(temp,"r");
    if (!elf_exec){
        tty_setcolor(COLOR_ERROR);
        tty_printf("[CMD] [EXEC] Программа `%s` не найдена в текущей папке.\n",v[1]);
        return 2;
    }
    //elf_info(temp);

    run_elf_file(temp, c, v);
    
    fclose(elf_exec);
    return 0;
}

uint32_t cmd_view(uint32_t argc, char* argv[]) {
	if(argc == 0) {
        tty_setcolor(COLOR_ERROR);
        tty_printf("[CMD] [VIEW] Для выполнения этой команды необходимо указать имя файла.\n");
		return 1;
	}

	struct DukeImageMeta* imdata = kmalloc(sizeof(DukeHeader_t));

    char* rpath = argv[1];

	FILE* fp = fopen(rpath, "r");

	if(!fp) {
        tty_setcolor(COLOR_ERROR);
        tty_printf("[CMD] [VIEW] Произошла ошибка при открытии файла %s (evaluated from %s)\n", rpath, argv[1]);
        kfree(rpath);
		//tty_printf("%s: Failed to open file!!!\n", argv[0]);
		return 1;
	}

	fread_c(fp, 1, sizeof(DukeHeader_t), imdata);
    fclose(fp);

	uint32_t w = getWidthScreen() - imdata->width;
	uint32_t h = getHeightScreen() - imdata->height;

	clean_tty_screen();

	char okay = duke_draw_from_file(rpath, w/2, h/2);

    if(!okay) {
        tty_setcolor(COLOR_ERROR);
        tty_printf("[CMD] [VIEW] Во время рендера картинки произошла ошибка.\n");
        //tty_printf("Error while drawing...\n");
    }

    kfree(rpath);
    kfree(imdata);
	return 0;
}

uint32_t cmd_meminfo() {
    size_t used = memory_get_used_kernel();
    size_t free = getInstalledRam() - used;

    char i, fill = 0;

    tty_printf("%c", '[');
    for(i = 0, fill = ((float)used/(float)KERNEL_HEAP_SIZE)*30; i < fill; i++) {
        _tty_printf("%c", '=');
    }
    for(i = 0, fill = 30-(((float)used/(float)KERNEL_HEAP_SIZE)*30); i < fill; i++) {
        _tty_printf("%c", ' ');
    }
    tty_printf("] %f%%\n", ((float)used/(float)KERNEL_HEAP_SIZE)*100);

    tty_printf("Used: %d kB (%d bytes)\n", used/1024, used);
    tty_printf("Free: %d kB (%d bytes)\n", free/1024, free);

    print_allocated_map();
    tty_printf("Additional information about heap also wriiten to COM1");
}

uint32_t cmd_sound(uint32_t c, char* v[]) {
    beeperInit(1);
    return 0;
}

Command_t shell_commands[] = {
	{"cat", cmd_cat, "Показать содержимое файла."},
	{"cd", cmd_cd, "Перейти в папку"},
	{"cls", cmd_cls, "Очистить экран"},
	{"cpuinfo", cmd_cpuinfo, "Информация о процессоре"},
	{"font", cmd_font, "Показать все глифы шрифта"},
	{"gui", (uint32_t (*)(uint32_t,  char **))parallel_desktop_start, "Запустить Parallel Desktop"},
	{"help", cmd_help, "Справка / Помощь"},
	{"hostname", cmd_hostname, "Показать или установить имя хоста"},
	{"ls", cmd_ls, "Показать содержимое каталога"},
	{"milla", cmd_milla, "Утилита для взаимодействия с Milla FS"},
	{"meminfo", cmd_meminfo, "Показать информацию о памяти (полезно для отладки)"},
	{"piano", (uint32_t (*)(uint32_t,  char **))piano, "Беги фортепиано (PC Speaker)"},
	{"reboot", cmd_reboot, "Перезагрузите систему"},
	{"shutdown", cmd_shutdown, "Выключить компьютер"},
	{"sound", cmd_sound, "Воспроизведение мелодии через PC Speaker"},
	// {"ac97", cmd_ac97, "Воспроизведение мелодии через AC97"},
	{"sysinfo", cmd_sysinfo, "Показать информацию о системе"},
	{"view", cmd_view, "Рисует картинку"},
	{"whoami", cmd_whoami, "Кто я?"},
	{NULL, NULL, NULL}  // Конец списка (EOL), спасибо Никите (pimnik98)!
};

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

        qemu_log("Argc total: %d", argc);
        for(size_t i = 0; argc >= i; i++){
            qemu_log("[CMD] '%s' => argc: %d => argv: %s", ncmd, i, argv[i]);
        }
        qemu_log("Check ok.");

        bool found = false;

        for(size_t i = 0; shell_commands[i].name != NULL; i++) {
            if(strcmp(shell_commands[i].name, argv[0]) == 0) {
                shell_commands[i].funcv(argc, argv);
                found = true;
                break;
            }
        }

        if(!found) {
            find_and_run_program(argc + 1, argv);
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
    char* ncmd = kcalloc(256, sizeof(char));
    tty_setcolor(COLOR_TEXT);

    while (1) {
        tty_setcolor(COLOR_SYS_TEXT);
        tty_printf("\n%s@%s:",getUserName(),getHostname());
        tty_setcolor(COLOR_SYS_PATH);
        tty_printf("%s$ ", getSysPath());

        tty_setcolor(COLOR_TEXT);

        memset(ncmd, 0, 256);

        gets(ncmd);
        if (strlen(ncmd) > 255) {
            tty_setcolor(COLOR_ERROR);
            tty_printf("\nERROR: limit 256 char's!");
            tty_printf("\nTODO: Dynamic strings for shell!");
            continue;
        }
        if(strlen(ncmd) == 0) {
            continue;
        }
        qemu_log("[CMD] '%s'",ncmd);
        cmdHandler(ncmd);
    }

    free(ncmd);
}

