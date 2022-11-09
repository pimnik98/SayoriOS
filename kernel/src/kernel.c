/**
 * @file kernel.c
 * @author Пиминов Никита (nikita.piminoff@yandex.ru)
 * @brief Основная точка входа в ядро
 * @version 0.3.0
 * @date 2022-11-01
 * @copyright Copyright SayoriOS Team (c) 2022
 */

#define KEYBOARD_TEST 0

#include "kernel.h"
#include "lib/stdio.h"
multiboot_header_t* multiboot;
uint32_t init_esp = 0;
bool initRD = false;

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
    qemu_log("Font system initialization...");
    tty_fontConfigurate();
    setColorFont(0xFFFFFF);
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
    sleep_ms(2500);
    init_syscalls();

    bootScreenPaint("Настройка I/O диспечера...");
    init_io_dispatcher();

    bootScreenPaint("Определение процессора...");
    detect_cpu(1);

    bootScreenPaint("Готово...");
    sleep_ms(250);
    bootScreenClose(0x000000,0xFFFFFF);
    tty_printf("SayoriOS v%d.%d.%d\nДата компиляции: %s\n",
        VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH,    // Версия ядра
        __TIMESTAMP__                                   // Время окончания компиляции ядра
    );
    tty_printf("\nВлюбиться можно в красоту, но полюбить - лишь только душу.\n(c) Уильям Шекспир\n");
    
    sayori_time_t time = get_time();
    tty_printf("\nВремя: %d:%d:%d\n", time.hours, time.minutes, time.seconds);
    keyboardInit();
    tty_taskInit();

	// TEST ZONE

	/*
	char buf[64] = {0};
	FILE* fo = fopen("/initrd/test.txt", "r");
		int ret = fread_c(fo, 5, 1, buf);
	tty_printf("File contents: %s\n", buf);

	*/
	
	/* WORKS -----v
	int tnode = vfs_foundMount("/initrd/test.txt");
	int telem = vfs_findFile("/initrd/test.txt");

	vfs_read(tnode, telem, 0, 5, buf);
	tty_printf("File contents: %s\n", buf);	
	*/

	// WORKS -----v
	//tty_printf("File contents: %s\n", fread(fopen("/initrd/test.txt", "r")));

    //int icode = run_elf_file("/start", 0, 0);
    //tty_printf("ELF Run Code: %d\n", icode);

    struct dirent* testFS = vfs_getListFolder("/");
    size_t sss = vfs_getCountElemDir("/");
    for(size_t f = 0;sss > f;f++){
        qemu_log("[%d] %s",testFS[f].ino,testFS[f].name);
    }
    qemu_log("[%d] %d [%s | %s]",sss,sizeof(testFS),testFS[0].name,testFS[1].name);

    #if KEYBOARD_TEST==1
    tty_printf("\n> Print any characters here. Go ahead!\n\n");
    keyboardctl(KEYBOARD_ECHO, false);
    while(1) {
        tty_printf(".%s", getCharKeyboardWait(false));
    }
    #endif

    shell();
    
    return 0;
}
