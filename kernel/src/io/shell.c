/**
 * @file io/shell.c
 * @author Пиминов Никита (nikita.piminoff@yandex.ru), NDRAEY >_ (pikachu_andrey@vk.com)
 * @brief Sayori Command Line (SCL -> Shell)
 * @version 0.3.3
 * @date 2022-10-20
 * @copyright Copyright SayoriOS Team (c) 2022-2023
 */
#include <io/ports.h>
#include <sys/trigger.h>
#include <io/tty.h>
#include <io/screen.h>
#include <sys/cpuinfo.h>
#include <fmt/tga.h>
#include <drv/audio/ac97.h>
#include <sys/memory.h>
#include <drv/vfs_new.h>
#include <io/duke_image.h>
#include <io/status_loggers.h>
#include <lib/stdio.h>
#include <fs/milla.h>
#include <gui/parallel_desktop.h>
#include "sys/cpuid.h"
#include "sys/pixfmt.h"

typedef struct {
	char* name;
	uint32_t (*funcv)(uint32_t, char**);
	char* helpstring;
} Command_t;

bool force_shell_closed = false;	///< Принудительное закрытие shell'a после выполнения команды
char* cmd = "";						///< Текущая команда
char* oldcmd = "";					///< Старая команда
size_t ShellKBD_i = 0;				///< Индекс триггера
Command_t shell_commands[];

void piano();
void minesweeper();

uint32_t cmd_crash() {
    // size_t a = 2/0;
    // detect_cpu(a);

    // a /= 0;

    *(char*)0xabcdef12 = 0;
    
    return 0;
}

uint32_t cmd_help(__attribute__((unused)) uint32_t c, __attribute__((unused)) char* v[]){
    size_t hlp_padding = 24;
    _tty_printf("Доступные команды:\n");

	for(size_t i = 0; shell_commands[i].name != nullptr; i++) {
        _tty_printf("\t%s", shell_commands[i].name);
        for(size_t j = 0; j < hlp_padding - strlen(shell_commands[i].name); j++) {
            _tty_printf(" ");
        }
        _tty_printf(" | %s\n", shell_commands[i].helpstring);
    }

	punch();

	return 0;
}

uint32_t cmd_cpuinfo(__attribute__((unused)) uint32_t c, __attribute__((unused)) char* v[]){
    detect_cpu(0);

	struct cpu_info info = cpu_get_basic_info();

	tty_printf("Features: ");
	for(int i = 0; i < 32; i++) {
		if(info.feature_flags_edx & (1 << i)) {
			_tty_printf("%s ", cpu_flag_edx_description[i]);
		}
	}
	tty_printf("\n");

	if(is_temperature_module_present())
		tty_printf("CPU Temperature: %d *C\n", get_cpu_temperature());

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
uint32_t cmd_reboot(__attribute__((unused)) uint32_t c, __attribute__((unused)) char* v[]){
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
uint32_t cmd_shutdown(__attribute__((unused)) uint32_t c, __attribute__((unused)) char* v[]){
    tty_printf("SHUTDOWN NOW!\n");
    sleep_ms(100);
    shutdown();
    return 0;
}

/**
 * @brief Вывод информации о системе
 */
void sysinfo(){
    clean_tty_screen();

	tga_header_t hdr = {};

	tga_extract_info("/Sayori/Images/sysinfo.tga", &hdr);

	size_t size = hdr.w * hdr.h * (hdr.bpp >> 3);

	uint32_t* buffer = kmalloc(size);
	tga_extract_pixels("/Sayori/Images/sysinfo.tga", buffer);

	pixfmt_conv((char*)buffer, hdr.bpp, hdr.w, hdr.h, SCREEN_BGR, SCREEN_RGB);

	duke_rawdraw2((const char*)buffer, hdr.h, hdr.w, hdr.bpp, 0, 0);

	kfree(buffer);
	//    duke_draw_from_file("/Sayori/Images/sysinfo.duke", 0, 0);


    setPosY(256);

    tty_printf("SayoriOS by SayoriOS Team (pimnik98 and NDRAEY)\n\n");

    sayori_time_t time = get_time();

    tty_printf("Системная информация:\n");
    tty_printf("\tOS:                      SayoriOS v%d.%d.%d\n", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);
    tty_printf("\tДата сборки:             %s\n", __TIMESTAMP__);
    tty_printf("\tАрхитектура:             %s\n", ARCH_TYPE);
    tty_printf("\tПроцессор:               %s\n", getNameBrand());
    tty_printf("\tОЗУ:                     %d kb\n", getInstalledRam()/1024);
    tty_printf("\tВидеоадаптер:            %s\n","Basic video adapter (Unknown)");
    tty_printf("\tДисплей:                 %s (%dx%d)\n", "(\?\?\?)", getScreenWidth(), getScreenHeight());
    tty_printf("\tТики:                    %d\n", getTicks());
    tty_printf("\tЧастота таймера:         %d Гц\n", getFrequency());
    tty_printf("\tВремя с момента запуска: %f секунд\n", getUptime());
    tty_printf("\tТекущее время:           %d:%d:%d\n", time.hours, time.minutes, time.seconds);
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
uint32_t cmd_sysinfo(__attribute__((unused)) uint32_t c, __attribute__((unused)) char* v[]){
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
    
    uint8_t buffer;

	while(ftell(cat_file) < filesize) {
		fread(cat_file, 1, 1, &buffer);
		tty_putchar(buffer, 0);
	}

    kfree(full_path);
    
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
uint32_t cmd_ac97(uint32_t c, char* v[]){
    if (c == 0){
        tty_setcolor(COLOR_ERROR);
        tty_printf("[CMD] [AC97] Необходимо указать путь для открытия файла.\n");
        return 1;
    }

    FILE* file = fopen(v[1], "rb");
    tty_printf("[CMD] [AC97] Попытка открытия файла %s...\n", v[1]);

    if (ferror(file) != 0){
        tty_setcolor(COLOR_ERROR);
        tty_printf("[CMD] [AC97] Не удалось найти файл `%s`. Проверьте правильность введенного вами пути.\n",v[1]);
        return 2;
    }

    fseek(file, 0, SEEK_END);

    uint32_t filesize = ftell(file);
    fseek(file, 0, SEEK_SET);

    ac97_set_master_volume(2, 2, false);
    ac97_set_pcm_volume(2, 2, false);

    char* data = kmalloc(filesize);
	fread(file, filesize, 1, data);

    char* dataptr = data;

    while(1) {
        size_t write_size = MIN(filesize, 65536);

        if(filesize == 0)
            break;

        size_t page_count = ac97_copy_user_memory_to_dma(dataptr, write_size);

        // for(ssize_t i = page_count; i > 0; i-= 32) {
        ac97_single_page_write_wait(page_count);
        // }

        dataptr += write_size;
        filesize -= write_size;
    }

    ac97_reset_channel();

    kfree(data);
    fclose(file);

    return 0;
}

/**
 * @brief Функция выводящая TGA
 *
 * @warning Функция сделана для консоли
 *
 * @param uint32_t c - Кол-во аргументов
 * @param char* v[] - Аргементы
 *
 * @return uint32_t - Результат работы
 */
uint32_t cmd_tga(uint32_t c, char* v[]){
    if (c == 0){
        tty_setcolor(COLOR_ERROR);
        tty_printf("[CMD] [TGA] Необходимо указать путь для открытия файла.\n");
        return 1;
    }
	tga_info(v[1]);
	tga_paint(v[1]);
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
	TExplorer();
    return 0;
}

/*
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
    // init = __milla_cleanState();
    // if (init != 1){
    //     tty_setcolor(COLOR_ERROR);
    //     tty_printf("[CMD] [Milla] Не удалось отчистить буфер Milla. Код ошибки: %d\n",init);
    //     return -2;
    // }

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
*/

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
    for(size_t f = 0; f < sss; f++){
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

	fread(fp, 1, sizeof(DukeHeader_t), imdata);
    fclose(fp);

	uint32_t w = getScreenWidth() - imdata->width;
	uint32_t h = getScreenHeight() - imdata->height;

	clean_tty_screen();

	bool error = duke_draw_from_file(rpath, w / 2, h / 2);

    if(error) {
        tty_setcolor(COLOR_ERROR);
        tty_printf("[CMD] [VIEW] Во время рендера картинки произошла ошибка.\n");
    }

    kfree(rpath);
    kfree(imdata);
	return 0;
}

uint32_t cmd_meminfo() {
    double used = (double)memory_get_used_kernel();
    double total = (double)getInstalledRam();
    double free = total - used;

    char i, fill = 0;

    tty_printf("%c", '[');
    for(i = 0, fill = (char)((used / total) * 30); i < fill; i++) {
        _tty_printf("%c", '=');
    }
    for(i = 0, fill = 30-((used / total) * 30); i < fill; i++) {
        _tty_printf("%c", ' ');
    }
    tty_printf("] %f%%\n", (used / total) * 100);

    tty_printf("Total: %d kB (%d bytes)\n", (size_t)total >> 10, (size_t)total);
    tty_printf("Used:  %d kB (%d bytes)\n", (size_t)used >> 10, (size_t)used);
    tty_printf("Free:  %d kB (%d bytes)\n", (size_t)free >> 10, (size_t)free);

    print_allocated_map();
    tty_printf("Additional information about heap also wriiten to COM1 (Make sure you're using DEBUG build)");

    return 0;
}

uint32_t cmd_sound(uint32_t c, char* v[]) {
    beeperInit(1);
    return 0;
}

uint32_t dan_view(uint32_t argc, char** argv);
uint32_t mala_draw(uint32_t argc, char** argv);
uint32_t calendar(uint32_t argc, char** argv);
uint32_t shell_diskctl(uint32_t argc, char** argv);
uint32_t dino_filemanager(uint32_t argc, char** argv);

uint32_t gbwrap(uint32_t argc, char** argv) {
    tty_printf("%d\n", argc);

    for(int i = 0; i < argc + 1; i++) {
        tty_printf("#%d = %s\n", i, argv[i]);
    }

    gb_main(argc + 1, argv);

	return 0;
}

Command_t shell_commands[] = {
	{"calendar", calendar, "Календарь"},
	{"cat", cmd_cat, "Показать содержимое файла."},
	{"cd", cmd_cd, "Перейти в папку"},
	{"cls", cmd_cls, "Очистить экран"},
	{"cpuinfo", cmd_cpuinfo, "Информация о процессоре"},
    {"crash", cmd_crash, "Crash the system"},
	{"danview", dan_view, "DAN Image viewer - Просмотр фото"},
	{"dino", dino_filemanager, "File manager (navigator)"},
	{"diskctl", shell_diskctl, "Disk utility"},
	{"font", cmd_font, "Показать все глифы шрифта"},
	{"gui", (uint32_t (*)(uint32_t,  char **))parallel_desktop_start, "Запустить Parallel Desktop"},
	{"help",     cmd_help,                                         "Справка / Помощь"},
	{"hostname", cmd_hostname,                                     "Показать или установить имя хоста"},
	{"ls",       cmd_ls,                                           "Показать содержимое каталога"},
	// {"milla", cmd_milla, "Утилита для взаимодействия с Milla FS"},
	{"meminfo",  cmd_meminfo,                                      "Показать информацию о памяти (полезно для отладки)"},
	{"mala",     mala_draw,                                        "Рисовалка"},
	{"piano",    (uint32_t (*)(uint32_t,  char **))piano,          "Беги фортепиано (PC Speaker)"},
	{"reboot",   cmd_reboot,                                       "Перезагрузите систему"},
	{"shutdown", cmd_shutdown,                                     "Выключить компьютер"},
	{"sound",    cmd_sound,                                        "Воспроизведение мелодии через PC Speaker"},
	{"pci",      (uint32_t (*)(uint32_t, char **)) pci_print_list, "PCI Devices"},
	{"play",     cmd_ac97,                                         "Воспроизведение мелодии через AC97"},
	{"tga",      cmd_tga,                                          "Targa"},
	{"sysinfo",  cmd_sysinfo,                                      "Показать информацию о системе"},
	{"view",     cmd_view,                                         "Рисует картинку"},
	{"whoami",   cmd_whoami,                                       "Кто я?"},
  {"gb",       gbwrap,                                           "GameBoy Emulator"},
  {"minesweeper", minesweeper,                                   "Сапёр"},
	{nullptr, nullptr, nullptr}  // Конец списка (EOL), спасибо Никите (pimnik98)!
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
		OffTrigger(ShellKBD_i);
		set_cursor_enabled(0);

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

        for(size_t i = 0; shell_commands[i].name != nullptr; i++) {
            if(strcmp(shell_commands[i].name, argv[0]) == 0) {
                shell_commands[i].funcv(argc, argv);
                found = true;
				
                break;
            }
        }

        if(!found) {
            find_and_run_program(argc + 1, argv);
        }
		
		set_cursor_enabled(1);
		OnTrigger(ShellKBD_i);
}

void ShellKBD(void* data1,void* data2,void* data3,void* data4,void* data5){
	if ((int) data1 == 82){
		OffTrigger(ShellKBD_i);
		set_cursor_enabled(0);
		tty_setcolor(COLOR_SYS_TEXT);
		tty_printf("\n%s@%s:",getUserName(),getHostname());
		tty_setcolor(COLOR_SYS_PATH);
		tty_printf("%s$ ", getSysPath());
		tty_setcolor(COLOR_TEXT);
		tty_printf("%s\n",oldcmd);
		cmdHandler(oldcmd);
		tty_setcolor(COLOR_SYS_PATH);
		tty_printf("* Команда выполнена. Нажмите 'Enter' чтобы вернуться в консоль.\n");
		tty_setcolor(COLOR_TEXT);
		set_cursor_enabled(1);
		OnTrigger(ShellKBD_i);
	}
	//qemu_log("[ShellKBD] Key:%d | Pressed: %x",(int) data1, (int) data2);
	
	/// Помещаем 0х0 для отключения дальнейшей обработки клавиатуры, но триггеры продолжат свою работу.
	//memcpy(data5,(int) (0x0),sizeof(int));
}


void ShellTimes(void* data1,void* data2,void* data3,void* data4,void* data5){
    char time_str[16] = {0};

	drawRect(730, 0, 8*8, 16, 0x000000);

	sayori_time_t time = get_time();

    // use sprintf!
    sprintf(time_str, "%s%d:%s%d:%s%d", (time.hours>10?"":"0"), time.hours, (time.minutes>10?"":"0"), time.minutes, (time.seconds>10?"":"0"), time.seconds);

    draw_vga_str(time_str, strlen(time_str), 730, 0, 0xffffff);

    sleep_ms(1000);
}

void shellForceClose(){
	force_shell_closed = true;
	qemu_log("SHELL FORCE CLOSED!");
	OffTrigger(ShellKBD_i);
	set_cursor_enabled(0);
}

/**
 * @brief Точка входа в консоль
 */
void shell(){
	force_shell_closed = false;
	ShellKBD_i = RegTrigger(0x0001, &ShellKBD);	///< Регистрация нажатий клавы, к примеру Insert
	
    //changeStageKeyboard(1);
    tty_set_bgcolor(COLOR_BG);
    tty_setcolor(COLOR_ALERT);
    tty_printf("\nВведите \"help\" для получения списка команд.\n");
    char* ncmd = kcalloc(256, sizeof(char));
    tty_setcolor(COLOR_TEXT);
    oldcmd = kmalloc(256);
    oldcmd = "";
    oldcmd[1] = '\0';
	
    // v---- BUG: Clock is always active, even in graphical apps like `Piano`, `Parallel Desktop`, `draw`, and so on...
	// RegTrigger(0x1111,&ShellTimes); ///< Отключена, из-за кривой реализации.

    while (1) {
		if (force_shell_closed) break;
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
            
            // Oh god, just rewrite shell in C++
            tty_printf("\nTODO: Dynamic strings for shell!\n");
            continue;
        }

        if(strlen(ncmd) == 0) {
            continue;
        }
        
        //qemu_log("[CMD] '%s'",ncmd);
		
        cmdHandler(ncmd);
		memcpy(oldcmd,ncmd,strlen(ncmd));
		oldcmd[strlen(ncmd)] = '\0';
		//qemu_log("OLDCMD: '%s'",oldcmd);
    }
	///> Эй, ты куда?
    // Туда
    kfree(ncmd);
}
