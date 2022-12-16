/**
 * @file kernel.c
 * @author Пиминов Никита (nikita.piminoff@yandex.ru) and Andrey (Drew) Pavlenko (pikachu.andrey@vk.com)
 * @brief Основная точка входа в ядро
 * @version 0.3.0
 * @date 2022-11-01
 * @copyright Copyright SayoriOS Team (c) 2022
 */

#define KEYBOARD_TEST 0

#include "kernel.h"
#include "lib/stdio.h"
#include "sys/float.h"
#include "sys/rsdp.h"
#include <io/colors.h>

multiboot_header_t* multiboot;
uint32_t init_esp = 0;
bool initRD = false;
bool autoexec = false;
char* cmd_autoexec = "";

/**
 * @brief Обработка комманд указаных ядру при загрузке
 *
 * @param char* cmd - Команды
 */
void kHandlerCMD(char* cmd){
    qemu_log("[kCMD] '%s'",cmd);
    uint32_t kCMDc = str_cdsp(cmd," ");
    uint32_t kCMDc_c = 0;
    char* out[128] = {0};
    str_split(cmd,out," ");
     for(int i = 0; kCMDc >= i; i++){
        kCMDc_c = str_cdsp(out[i],"=");
        char* out_data[128] = {0};
        if (kCMDc_c != 1){
            qemu_log("[kCMD] [%d] %s is ignore.",i,out[i]);
            continue;
        }
        str_split(out[i],out_data,"=");
        if (strcmpn(out_data[0],"bootscreen")){
            // Config BOOTSCREEN
            if (strcmpn(out_data[1],"minimal")){
                bootScreenChangeMode(1);
            } else if (strcmpn(out_data[1],"light")){
                bootScreenChangeTheme(1);
            } else if (strcmpn(out_data[1],"dark")){
                bootScreenChangeTheme(0);
            } else {
                qemu_log("\t Sorry, no support bootscreen mode!");
            }
        }
        if (strcmpn(out_data[0],"exec")){
            cmd_autoexec = out_data[1];
            autoexec = true;
            qemu_log("\t After the kernel has fully started, the `%s` program will be launched",cmd_autoexec);
        }
        //qemu_log("[kCMD] [%d] %s >\n\tKey: %s\n\tValue:%s",i,out[i],out_data[0],out_data[1]);
     }
}

/**
 * @brief Монтирует виртуальный диск с файловой системой Sayori Easy File System
 *
 * @param int irdst - Точка монтирования
 */
void initrd_sefs(int irdst){
    if (initRD){
        return;
    }
    qemu_log("[InitRD] [SEFS] Initialization of the virtual disk. The SEFS virtual file system is used.");
    qemu_log("[InitRD] [SEFS] The virtual disk space is located at address %x.",irdst);
    vfs_reg(irdst,VFS_TYPE_MOUNT_SEFS);
    initRD = true;
}


/**
 * @brief Инициализирует модули подключенные к ОС
 *
 */
void kModules_Init(){
    qemu_log("[kModules] Loading operating system modules...");
    uint32_t*	mod_start = 0;
    uint32_t*	mod_end = 0;
    uint32_t	mods_count = multiboot->mods_count;
    char mod_cmd[16][64] = {0};
    if (mods_count > 0){
		mod_start = (uint32_t*) kmalloc(sizeof(uint32_t)*mods_count);
		mod_end = (uint32_t*) kmalloc(sizeof(uint32_t)*mods_count);
        qemu_log("[kModules] Found '%d' modules",mods_count);
		for (size_t i = 0; i < mods_count; i++){
			mod_start[i] = *(uint32_t*)(multiboot->mods_addr + 8*i);
			mod_end[i] = *(uint32_t*)(multiboot->mods_addr + 8*i + 4);
            multiboot_module_t *mod = (multiboot_module_t *) (uint32_t*)(multiboot->mods_addr + 8*i);
            strcpy(mod_cmd[i],(char *) mod->cmdline);
            qemu_log("[kModules] Found module number `%d`. (Start: %x | End: %x) CMD: %s",i,mod_start[i],mod_end[i],mod_cmd[i]);
            if (strcmpn(mod_cmd[i],"initrd_sefs")){
                initrd_sefs(mod_start[i]);
                continue;
            }
		}
	} else {
        qemu_log("[kModules] No modules were connected to this operating system.");
    }
}

/**
 * @brief Точка входа в ядро
 *
 * @param multiboot_header_t irdst - Информация MultiBoot
 * @param uint32_t initial_esp -  Точка входа
 */
int kernel(multiboot_header_t* mboot, uint32_t initial_esp){
    multiboot = mboot;
    drawASCIILogo(0);
    qemu_log("SayoriOS v%d.%d.%d\nBuilt: %s",
        VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH,    // Версия ядра
        __TIMESTAMP__                                   // Время окончания компиляции ядра
    );
    kHandlerCMD(mboot->cmdline);
    qemu_log("Setting `Interrupt Descriptor Table`...");
    init_descriptor_tables();
    qemu_log("Setting `RIH`...");
    isr_init();
    qemu_log("Checking RAM...");
    check_memory_map((memory_map_entry_t*) mboot->mmap_addr, mboot->mmap_length);
    qemu_log("Memory manager initialization...");
    init_memory_manager(initial_esp);

    kModules_Init();
    setFontPath("/var/fonts/fonts.duke","/var/fonts/fonts.fdat"); // Для 9го размера
    setConfigurationFont(6,10,12); // Для 9
    qemu_log("Initializing the virtual video memory manager...");
    init_vbe(mboot);
    fontInit();
    qemu_log("Initalizing fonts...");
    tty_fontConfigurate();
    setColorFont(0xFFFFFF);

    qemu_log("Initializing FPU...");
	fpu_init();

    bootScreenInit(7);
    bootScreenLazy(true);

    bootScreenPaint("Настройка таймеров...");
    init_timer(BASE_FREQ);
    asm volatile ("sti");

    bootScreenPaint("Тестирование пищалки...");
    beeperInit(0);

    bootScreenPaint("Настройка менеджера задач...");
    qemu_log("Registering a Task Manager...");
    init_task_manager();

    bootScreenPaint("Настройка системных вызовов...");
    qemu_log("Registering System Calls...");
    // sleep_ms(2500);
    init_syscalls();

    bootScreenPaint("Настройка ENV...");
    qemu_log("Registering ENV...");
    confidEnv();

    bootScreenPaint("Настройка I/O диспечера...");
    qemu_log("Init I/O dispatcher...");
    init_io_dispatcher();

    bootScreenPaint("Определение процессора...");
    detect_cpu(1);

    bootScreenPaint("Готово...");
    sleep_ms(250);
    bootScreenClose(0x000000,0xFFFFFF);
    tty_set_bgcolor(COLOR_BG);
    tty_printf("SayoriOS v%d.%d.%d\nДата компиляции: %s\n",
        VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH,    // Версия ядра
        __TIMESTAMP__                                   // Время окончания компиляции ядра
    );
    tty_printf("\nВлюбиться можно в красоту, но полюбить - лишь только душу.\n(c) Уильям Шекспир\n");
    
    sayori_time_t time = get_time();
    tty_printf("\nВремя: %d:%d:%d\n", time.hours, time.minutes, time.seconds);
    keyboardInit();
    mouse_install();

    tty_taskInit();
    qemu_log("Initialized cursor animation...");

    if (autoexec){
        // Данное условие сработает, если указан параметр ядра exec
        FILE* elf_auto = fopen(cmd_autoexec,"r");
        if (ferror(elf_auto) != 0){
            qemu_log("Autorun: Программа `%s` не найдена.\n",cmd_autoexec);
        } else {
            run_elf_file(cmd_autoexec, 0, 0);
        }
    }
    
	// TEST ZONE

    struct dirent* testFS = vfs_getListFolder("/");
    size_t sss = vfs_getCountElemDir("/");
    for(size_t f = 0;sss > f;f++){
        qemu_log("[%d] %s",testFS[f].ino,testFS[f].name);
    }
    qemu_log("[%d] %d [%s | %s]",sss,sizeof(testFS),testFS[0].name,testFS[1].name);

	// tty_printf("Finding RSDP...\n");

    // int saddr;

	// for(saddr = 0x000E0000; saddr < 0x000FFFFF; saddr++) {
	// 	if(memcmp(saddr, rsdp_ptr, 8) == 0) {
	// 		tty_printf("Found! At: %x\n", saddr);
    //         break;
	// 	}
	// }

    // RSDPDescriptor* rsdp = saddr;
    // tty_printf("RSDP sig: %s\n", rsdp->signature);
    // tty_printf("RSDP checksum: %d\n", rsdp->checksum);
    // tty_printf("RSDP OEMID: %s\n", rsdp->OEMID);
    // tty_printf("RSDP revision: %d\n", rsdp->revision);
    // tty_printf("RSDP address: %x\n", rsdp->RSDTaddress);

    shell();
    
    return 0;
}
