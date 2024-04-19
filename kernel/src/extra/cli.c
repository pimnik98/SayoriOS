/**
 * @file extra/cli.c
 * @author Пиминов Никита (nikita.piminoff@yandex.ru), NDRAEY >_ (pikachu_andrey@vk.com)
 * @brief [CLI] Sayori Command Line (SCL -> Shell)
 * @version 0.3.5
 * @date 2022-10-20
 * @copyright Copyright SayoriOS Team (c) 2022-2024
 */

#include <io/ports.h>
#include <sys/variable.h>
#include "elf/elf.h"
#include "io/tty.h"
#include "mem/vmm.h"
#include "io/status_loggers.h"
#include "mem/pmm.h"
#include "lib/split.h"
#include "version.h"
#include "drv/input/keyboard.h"
#include "lib/php/explode.h"
#include "fs/nvfs.h"
#include "lib/list.h"
#include "sys/scheduler.h"
#include "sys/timer.h"
#include "drv/disk/dpm.h"
#include <fmt/tga.h>
#include "sys/pixfmt.h"
#include "io/rgb_image.h"
#include <sys/cpuinfo.h>
#include "../../include/lib/fileio.h"
#include "sys/system.h"
#include "debug/hexview.h"

int G_CLI_CURINXA = 0;
int G_CLI_CURINXB = 0;
int G_CLI_H_KYB = 1;
int G_CLI_CURINXD = 17;				///< Текущий диск
char G_CLI_PATH[1024] = "R:\\Sayori\\";

typedef struct {
	char* name;
	char* alias;
	uint32_t (*funcv)(uint32_t, char**);
	char* helpstring;
} CLI_CMD_ELEM;

CLI_CMD_ELEM G_CLI_CMD[];

// void F_CLI_KYB(void* data1,void* data2,void* data3,void* data4,void* data5){
// 	if (G_CLI_H_KYB == 0) return;
// 	qemu_log("[F_CLI_KYB] Key:%d | Pressed: %x",(int) data1, (int) data2);	
// }

uint32_t CLI_CMD_GBA(uint32_t c, char* v[]){
    if (c == 0 || (c == 1 && (strcmpn(v[1],"/?")))){
        _tty_printf("Эмулятор GameBoy.\n");
        _tty_printf("Пример:\"GBA R:\\game.gb\".\n");
        _tty_printf("\n");
        return 1;
    }
    // gb game.gb
    tty_printf("%d\n", c);

    for(int i = 0; i < c; i++) {
        tty_printf("#%d = %s\n", i, v[i]);
    }

    gb_main(c, v);

	return 1;
}


uint32_t CLI_CMD_CLS(uint32_t c, char* v[]){
    clean_tty_screen();
    return 1;
}

uint32_t CLI_CMD_SYSINFO(uint32_t c, char* v[]){
    clean_tty_screen();

    setPosY(256);

    tty_printf("SayoriOS by SayoriOS Team (pimnik98 and NDRAEY)\n\n");

    tty_printf("Системная информация:\n");
    tty_printf("\tOS:                      SayoriOS v%d.%d.%d\n", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);
    tty_printf("\tДата сборки:             %s\n", __TIMESTAMP__);
    tty_printf("\tАрхитектура:             %s\n", ARCH_TYPE);
    tty_printf("\tПроцессор:               %s\n", getNameBrand());

    if(is_temperature_module_present()) {
        tty_printf("\tТемпература:             %d *C\n", get_cpu_temperature());
    } else {
        tty_printf("\tТемпература:             -- *C\n");
    }

    tty_printf("\tОЗУ:                     %u kb\n", getInstalledRam()/1024);
    tty_printf("\tВидеоадаптер:            %s\n", "Legacy framebuffer (Unknown)");
    tty_printf("\tДисплей:                 %s (%dx%d)\n", "(\?\?\?)", getScreenWidth(), getScreenHeight());
    tty_printf("\tТики:                    %d\n", getTicks());
    tty_printf("\tЧастота таймера:         %d Гц\n", getFrequency());
    tty_printf("\tВремя с момента запуска: %f секунд\n", getUptime());
    return 1;
}


uint32_t CLI_CMD_DISKPART(uint32_t c, char* v[]){
    _tty_printf("Список примонтированных дисков:\n");
//    fsm_dpm_update(-1);
    for(int i = 0; i < 26; i++){
        DPM_Disk dpm = dpm_info(i + 65);
        if (dpm.Ready != 1) continue;
        _tty_printf(" [%c] %s | %s\n", i + 65, dpm.FileSystem, dpm.Name);
    }

    _tty_printf("\n");
    //clean_tty_screen();
    return 1;
}


uint32_t CLI_CMD_CAT(uint32_t c, char* v[]){
    if (c == 0 || (c == 1 && (strcmpn(v[1],"/?")))){
        _tty_printf("Данная программа выводит содержимое файла.\n");
        _tty_printf("Пример:\"CAT R:\\Sayori\\motd\".\n");
        _tty_printf("\n");
        return 1;
    }
    FILE* cat_file = fopen(v[1], "r");
    if (!cat_file){
        tty_setcolor(COLOR_ERROR);
        tty_printf("[CMD] [CAT] Не удалось найти файл `%s`. Проверьте правильность введенного вами пути.\n",v[1]);
        return 2;
    }

    size_t filesize = fsize(cat_file);

    uint8_t* buffer = kcalloc(1,filesize + 1);

    fread(cat_file, 1, filesize, buffer);

	tty_printf("%s", buffer);

    fclose(cat_file);

	kfree(buffer);
    return 1;
}

uint32_t CLI_CMD_DEL(uint32_t c, char* v[]){
    if (c == 0 || (c == 1 && (strcmpn(v[1],"/?")))){
        _tty_printf("Удаление файла\n");
        _tty_printf("Пример:\"DEL T:\\Sayori\\tmp.log\".\n");
        _tty_printf("\n");
        return 1;
    }

    bool res = unlink(v[1]);

    if (!res) {
        tty_setcolor(COLOR_ERROR);
        tty_printf("Не удалось удалить файл, возможно файл не найден или у вас недостаточно прав для его удаления.\n");
        return 1;
    }
    return 0;
}


uint32_t CLI_CMD_RMDIR(uint32_t c, char* v[]){
    if (c == 0 || (c == 1 && (strcmpn(v[1],"/?")))){
        _tty_printf("Удаление папки\n");
        _tty_printf("Пример:\"RMDIR T:\\Sayori\\\".\n");
        _tty_printf("\n");
        return 1;
    }

    bool res = rmdir(v[1]);

    if (!res) {
        tty_setcolor(COLOR_ERROR);
        tty_printf("Не удалось удалить папку, возможно папка не найдена или у вас недостаточно прав для её удаления.\n");
        return 1;
    }
    return 0;
}

uint32_t CLI_CMD_TOUCH(uint32_t c, char* v[]){
    if (c == 0 || (c == 1 && (strcmpn(v[1],"/?")))){
        _tty_printf("Создание файла\n");
        _tty_printf("Пример:\"TOUCH T:\\Sayori\\tmp.log\".\n");
        _tty_printf("\n");
        return 1;
    }

    bool res = touch(v[1]);

    if (!res) {
        tty_setcolor(COLOR_ERROR);
        tty_printf("Не удалось создать файл, возможно файл уже существует или у вас недостаточно прав для её создания в этой папке.\n");
        return 1;
    }
    return 0;
}

uint32_t CLI_CMD_MKDIR(uint32_t c, char* v[]){
    if (c == 0 || (c == 1 && (strcmpn(v[1],"/?")))){
        _tty_printf("Создание папки\n");
        _tty_printf("Пример:\"TOUCH T:\\Sayori\\\".\n");
        _tty_printf("\n");
        return 1;
    }

    bool res = mkdir(v[1]);

    if (!res) {
        tty_setcolor(COLOR_ERROR);
        tty_printf("Не удалось создать папка, возможно папка уже существует или у вас недостаточно прав для её создания в этой папке.\n");
        return 1;
    }
    return 0;
}

uint32_t CLI_CMD_JSE(uint32_t c, char* v[]){
    if (c == 0 || (c == 1 && (strcmpn(v[1],"/?")))){
        _tty_printf("JavaScript Engine.\n");
        _tty_printf("Пример:\"JSE R:\\jse\\console.js\".\n");
        _tty_printf("\n");
        return 1;
    }

    int res = elk_file(v[1]);

    if (!res) {
        tty_setcolor(COLOR_ERROR);
        tty_printf("   [JSE] Произошла ошибка при выполнении скрипта. Подробности отправлены в консоль.");
    }
    return 1;
}

uint32_t CLI_CMD_SET(uint32_t c, char* v[]){
	if (c == 1 && (strcmpn(v[1],"/?"))){
		_tty_printf("Для получения данных переменной введите \"SET ПЕРЕМЕННАЯ\".\n");
		_tty_printf("Для установки переменной введите \"SET ПЕРЕМЕННАЯ=ЗНАЧЕНИЕ\".\n");
		_tty_printf("Для удаления переменной введите \"SET ПЕРЕМЕННАЯ=\".\n\n");

		_tty_printf("\n");
		return 1;
	}
	if (c == 0){
		VARIABLE* s = variable_list("");
 		for(size_t i = 0; i<512; i++) {
 			if (s[i].Ready != 1) break;
 			_tty_printf("%s=%s\n", s[i].Key, s[i].Value);
 		}
		kfree(s);
	} else {
		uint32_t pc = str_cdsp2(v[1], '=');
		char** out = explode(v[1], '=');
	
		if (pc != 1){
			/// Поиск переменной
			qemu_log("[Режим поиска] %s",v[1]);
			VARIABLE* s = variable_list(v[1]);
			for(size_t i = 0; i<512; i++) {
				if (s[i].Ready != 1) break;
				_tty_printf("%s=%s\n", s[i].Key, s[i].Value);
			}
			kfree(s);
		} else if (out[1] == NULL || strlen(out[1]) == 0) {
			qemu_log("[Режим удаления] %s",out[0]);
			variable_write(out[0],"");
		} else if (out[1] != NULL) {
			qemu_log("[Режим изменения] %s",out[0]);
			variable_write(out[0],out[1]);
		}

// 		Почему-то не работает, а должно
// 		for (int d = 0; d <= out;d++){
// 			kfree(out[d]);
// 		}
 		kfree(out);
	}
	punch();

	return 0;
}

uint32_t CLI_CMD_DIR(uint32_t c, char* v[]) {
    const char* path = (c <= 1 ? G_CLI_PATH : v[1]);

	FSM_DIR* Dir = nvfs_dir(path);
	if (Dir->Ready != 1){
		tty_printf("Ошибка %d! При чтении папки: %s\n",Dir->Ready, path);
        kfree(Dir);
        return 1;
	} else {
		tty_printf("Содержимое папки папки: %s\n\n", path);
		size_t Sizes = 0;
		for (int i = 0; i < Dir->Count; i++){
			char* btime = fsm_timePrintable(Dir->Files[i].LastTime);
			tty_printf("%s\t%s\t\t%s\n", 
				btime,
				(Dir->Files[i].Type == 5?"<ПАПКА>":"<ФАЙЛ> "), 
				Dir->Files[i].Name
			);
			Sizes += Dir->Files[i].Size;
			kfree(btime);
		}
		tty_printf("\nФайлов: %d | Папок: %d | Всего: %d\n", Dir->CountFiles, Dir->CountDir, Dir->Count);
		tty_printf("Размер папки: %d мб. | %d кб. | %d б.\n", (Sizes != 0?(Sizes/1024/1024):0), (Sizes != 0?(Sizes/1024):0), Sizes);
	}
	/// Сначала чистим массив внутри массива
	kfree(Dir->Files);
	/// А потом только основной массив
	kfree(Dir);
	return 1;
}

uint32_t CLI_CMD_RUN(uint32_t c, char* v[]) {
    if (c == 0){
        //tty_setcolor(COLOR_ERROR);
        tty_printf("Файл не указан.\n");
        return 1;
    }

    const char* path = v[0];

    FILE* elf_exec = fopen(path, "r");

    if(!elf_exec) {
        fclose(elf_exec);
        tty_error("\"%s\" не является внутренней или внешней\n командой, исполняемой программой или пакетным файлом.\n", path);
        return 2;
    }

    if(!is_elf_file(elf_exec)) {
        fclose(elf_exec);
        tty_printf("\"%s\" не является программой или данный тип файла не поддерживается.\n", path);
        return 2;
    }

    fclose(elf_exec);

    run_elf_file(path, c, v);

    return 0;
}

uint32_t CLI_CMD_ECHO(uint32_t c, char* v[]){
	if (c == 1 && (strcmpn(v[1],"/?"))){
		_tty_printf("Данная команда выводит сообщение на экран, а также переменные.\n");
		_tty_printf("\n");
		return 1;
	}
	for (int i = 1; i <= c;i++){
		/// Сначало переформируем все в переменные в текст
		size_t len_v = strlen(v[i]);
		size_t len_e = len_v-1;
		if (v[i][0] == '%' && v[i][len_e] == '%'){
			char* tmp_ve = kmalloc(len_v);
			substr(tmp_ve,v[i],1,len_e-1);
			char* tmp_dv = variable_read(tmp_ve);
			kfree(tmp_ve);
			if (tmp_dv != NULL) {
				_tty_printf("%s",tmp_dv);
				continue;
			}
		}
		//if (str_contains(v[i],""))
		if (strcmpn(v[i],"%DATE%") || strcmpn(v[i],"%date%")){
			_tty_printf("2023-01-01");
		} else if (strcmpn(v[i],"%CD%") || strcmpn(v[i],"%cd%") || strcmpn(v[i],"%path%") || strcmpn(v[i],"%PATH%")){
			_tty_printf("%s",G_CLI_PATH);
		} else if (strcmpn(v[i],"%RANDOM%") || strcmpn(v[i],"%random%")){
			/// Магии не будет - я хз как у нас тут работает рандом
			_tty_printf("%d",1);
		} else if (strcmpn(v[i],"%TIME%") || strcmpn(v[i],"%time%")){
			_tty_printf("%s","12:34");
		} else {
			_tty_printf("%s ", v[i]);
		}
	}
	_tty_printf("\n");
	return 1;
}

uint32_t CLI_CMD_HELP(__attribute__((unused)) uint32_t c, __attribute__((unused)) char* v[]){
	_tty_printf("Для получения дополнительной информации, наберите \"команда /?\", если справка по команде есть, она будет отображена.\n\n");
    size_t hlp_padding = 11;
	for(size_t i = 0; G_CLI_CMD[i].name != nullptr; i++) {
        _tty_printf("%s", G_CLI_CMD[i].name);
        for(size_t j = 0; j < hlp_padding - strlen(G_CLI_CMD[i].name); j++) {
            _tty_printf(" ");
        }
        _tty_printf(" | %s\n", G_CLI_CMD[i].helpstring);
    }

	punch();

	return 1;
}

// Pimnik98, don't being thirsty

uint32_t gfxbench(uint32_t argc, char* args[]);
uint32_t miniplay(uint32_t argc, char* args[]);
uint32_t CLI_CMD_NET(uint32_t c, char **v);
uint32_t parallel_desktop_start(uint32_t argc, char* args[]);
uint32_t mala_draw(uint32_t argc, char* argv[]);
uint32_t pci_print_list(uint32_t argc, char* argv[]);
//uint32_t pavi_view(uint32_t argc, char* argv[]);
uint32_t rust_command(uint32_t argc, char* argv[]);
uint32_t CLI_MEMINFO(uint32_t argc, char* argv[]) {
	tty_printf("Физическая:\n");
	tty_printf("    Используется: %d байт (%d MB)\n", used_phys_memory_size, used_phys_memory_size / MB);
	tty_printf("    Свободно: %d байт (%d MB)\n",  phys_memory_size - used_phys_memory_size, (phys_memory_size - used_phys_memory_size) / MB);
	tty_printf("Виртуальная:\n");
	tty_printf("    %d записей\n", system_heap.allocated_count);
	tty_printf("    Используется: %d байт (%d MB)\n", system_heap.used_memory, system_heap.used_memory / MB);

	return 0;
}

uint32_t proc_list(uint32_t argc, char* argv[]) {
    extern list_t process_list;
    extern list_t thread_list;

    tty_printf("%d процессов\n", process_list.count);

    list_item_t* item = process_list.first;
    for(int i = 0; i < process_list.count; i++) {

        process_t* proc =  (process_t*)item;

        tty_printf("    Процесс: %d [%s]\n", proc->pid, proc->name);

        item = item->next;
    }

    tty_printf("%d потоков\n", thread_list.count);

    list_item_t* item_thread = thread_list.first;
    for(int j = 0; j < thread_list.count; j++) {
        thread_t* thread = (thread_t*)item_thread;

        tty_printf("    Поток: %d [Стек: (%x, %x, %d)]\n", thread->id, thread->stack_top, thread->stack, thread->stack_size);

        item_thread = item_thread->next;
    }

    return 0;
}

uint32_t CLI_CMD_REBOOT(uint32_t argc, char* argv[]) {
    reboot();

    return 0;
}

uint32_t CLI_RD(uint32_t argc, char* argv[]) {
    if(argc < 2) {
        tty_error("No arguments.\n");
        return 1;
    }

    char* disk = argv[1];
    DPM_Disk data = dpm_info(disk[0]);

    if(!data.Ready) {
        tty_error("No disk.\n");
        return 1;
    }

    char* newdata = kcalloc(1024, 1);

    dpm_read(disk[0], 0, 1024, newdata);

    hexview_advanced(newdata, 1024, 26, true, _tty_printf);

    punch();

    kfree(newdata);

    return 0;
}

uint32_t pavi_view(uint32_t, char**);
uint32_t minesweeper(uint32_t, char**);
uint32_t shell_diskctl(uint32_t, char**);
uint32_t calendar(uint32_t, char**);

CLI_CMD_ELEM G_CLI_CMD[] = {
	{"CLS", "cls", CLI_CMD_CLS, "Очистка экрана"},
    {"CALENDAR", "calendar", calendar, "Календарь"},
    {"CAT", "cat", CLI_CMD_CAT, "Выводит содержимое файла на экран"},
	{"ECHO", "echo", CLI_CMD_ECHO, "Выводит сообщение на экран."},
	{"DIR", "dir", CLI_CMD_DIR, "Выводит список файлов и папок."},
    {"DISKCTL", "diskctl", shell_diskctl, "Управление ATA-дисками"},
    {"DISKPART", "diskpart", CLI_CMD_DISKPART, "Список дисков Disk Partition Manager"},
    {"GBA", "gba", CLI_CMD_GBA, "GameBoy Emulator"},
	{"HELP", "help", CLI_CMD_HELP, "Выводит справочную информацию о командах SayoriOS (CLI)."},
	{"SET", "set", CLI_CMD_SET, "Показывает, указывает и удаляет переменные среды SayoriOS"},
	{"NET", "net", CLI_CMD_NET, "Информация об сетевых устройствах"},
	{"GFXBENCH", "gfxbench", gfxbench, "Тестирование скорости фреймбуфера"},
	{"MEMINFO", "meminfo", CLI_MEMINFO, "Информация об оперативной памяти"},
	{"MINIPLAY", "miniplay", miniplay, "WAV-проиграватель"},
	{"DESKTOP", "desktop", parallel_desktop_start, "Рабочий стол"},
	{"MALA", "mala", mala_draw, "Нарисовать рисунок"},
    {"MINESWEEPER", "minesweeper", minesweeper, "Сапёр"},
	{"PAVI", "pavi", pavi_view, "Программа для просмотра изображений"},
	{"PCI", "pci", pci_print_list, "Список PCI устройств"},
	// {"RS", "rs", rust_command, "Rust command"},
	{"PROC", "proc", proc_list, "Список процессов"},
    {"SYSINFO", "sysinfo", CLI_CMD_SYSINFO, "Информация о системе"},
    {"JSE", "jse", CLI_CMD_JSE, "JavaScript Engine"},
    {"TOUCH", "touch", CLI_CMD_TOUCH, "Создать файл"},
    {"DEL", "DEL", CLI_CMD_DEL, "Удалить файл"},
    {"MKDIR", "mkdir", CLI_CMD_MKDIR, "Создать папку"},
    {"RMDIR", "rmdir", CLI_CMD_RMDIR, "Удалить папку"},
    {"REBOOT", "reboot", CLI_CMD_REBOOT, "Перезагрузка"},
    {"RD", "rd", CLI_RD, "Чтение данных с диска"},
	{nullptr, nullptr, nullptr}
};

void cli_handler(const char* ncmd){
	set_cursor_enabled(0);

	uint32_t argc = str_cdsp(ncmd," ") + 1;
    char* argv[128] = {0};

    str_split(ncmd, argv, " ");

	for(size_t i = 0; i < argc; i++){
		qemu_log("[CLI] '%s' => argc: %d => argv: %s", ncmd, i, argv[i]);
    }

	bool found = false;

	for(size_t i = 0; G_CLI_CMD[i].name != nullptr; i++) {
		if(strcmpn(G_CLI_CMD[i].name, argv[0]) || strcmpn(G_CLI_CMD[i].alias, argv[0])) {
			G_CLI_CMD[i].funcv(argc, argv);
			found = true;
			break;
		}
	}
	if(!found) {
		CLI_CMD_RUN(argc + 1, argv);
	}

	set_cursor_enabled(1);
}

void cli(){
	qemu_log("[CLI] Started...");
	tty_set_bgcolor(0xFF000000);
    tty_setcolor(0xFFFFFF);

	variable_write("HOSTNAME", "SAYORISOUL");
	variable_write("SYSTEMROOT", "R:\\Sayori\\");
	variable_write("TEMP", "T:\\");
	variable_write("USERNAME", "OEM");

// 	T_CLI_KYB = RegTrigger(0x0001, &F_CLI_KYB);
	
//	clean_tty_screen();
	_tty_printf("SayoriOS [Версия: v%d.%d.%d]\n",VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);
	_tty_printf("(c) SayoriOS Team, 2023.\nДля дополнительной информации наберите \"help\".\n\n");

	punch();

	char* input_buffer = kcalloc(1, 256);
	while(1) {
		size_t memory_cur = system_heap.used_memory;
        size_t memory_cnt_cur = system_heap.allocated_count;

    	tty_setcolor(0xFFFFFF);
		tty_printf("%s>", G_CLI_PATH);
		memset(input_buffer, 0, 256);

        int result = gets_max(input_buffer, 255);

        if(result == 1) {
            tty_alert("\nMaximum 255 characters!\n");
            continue;
        }

        size_t len_cmd = strlen(input_buffer);
        if (len_cmd == 0) {
            continue;
        }

        size_t current_time = timestamp();
		qemu_log("cmd: %s", input_buffer);

		cli_handler(input_buffer);
		tty_printf("\n");

		ssize_t delta = (int)system_heap.used_memory - (int)memory_cur;
		ssize_t delta_blocks = (int)system_heap.allocated_count - (int)memory_cnt_cur;
		qemu_warn("Used memory before: %d (%d blocks)", memory_cur, memory_cnt_cur);
		qemu_warn("Used memory now: %d (%d blocks)", system_heap.used_memory, system_heap.allocated_count);
		qemu_warn("Memory used: %d (%d blocks)", delta, delta_blocks);

		if(delta > 0) {
			qemu_err("Memory leak!");
		}

        qemu_note("Time elapsed: %d milliseconds", timestamp() - current_time);
	}

    kfree(input_buffer);
}
