/**
 * @file kernel.c
 * @author Пиминов Никита (nikita.piminoff@yandex.ru), Drew >_ (pikachu_andrey@vk.com)
 * @brief Основная точка входа в ядро
 * @version 0.3.2
 * @date 2022-11-01
 * @copyright Copyright SayoriOS Team (c) 2022-2023
 */

#include "kernel.h"
#include "lib/stdio.h"
#include "sys/float.h"
#include "sys/rsdp.h"
#include <io/colors.h>

multiboot_header_t* multiboot;
uint32_t init_esp = 0;
bool initRD = false;
bool autoexec = false;
bool run_gui_at_startup = false;
char* cmd_autoexec = "";
bool test_ac97 = true;
bool test_pcs = true;

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
            } else if (strcmpn(out_data[1],"no-logs")){
                bootScreenLogs(false);
            } else {
                qemu_log("\t Sorry, no support bootscreen mode!");
            }
        }

        if (strcmpn(out_data[0],"NatSuki-Login")){
            __milla_setLogin(out_data[1]);
        }
        if (strcmpn(out_data[0],"NatSuki-Password")){
            __milla_setPasswd(out_data[1]);
        }
        if (strcmpn(out_data[0],"disable")){
            if (strcmpn(out_data[1],"coms")){
                __com_setInit(1,0);
                __com_setInit(2,0);
                __com_setInit(3,0);
                __com_setInit(4,0);
                qemu_log("\t COM-OUT DISABLED");
            } else if (strcmpn(out_data[1],"ac97")){
                test_ac97 = false;
                qemu_log("\t AC97 DISABLED");
            } else if (strcmpn(out_data[1],"pc-speaker")){
                test_pcs = false;
                qemu_log("\t PC-Speaker DISABLED");
            }  else {
                qemu_log("\t Sorry, no support!");
            }
        }

        if (strcmpn(out_data[0], "exec")){
            run_gui_at_startup = false;
            qemu_log("\t After the kernel has fully started, GUI will be launched");
        }
        if (strcmpn(out_data[0], "gui")){
            run_gui_at_startup = true;
            qemu_log("\t After the kernel has fully started, GUI will be launched");
        }
        //qemu_log("[kCMD] [%d] %s >\n\tKey: %s\n\tValue:%s",i,out[i],out_data[0],out_data[1]);
     }
}

/**
 * @brief Монтирует виртуальный диск с файловой системой Sayori Easy File System
 *
 * @param int irdst - Точка монтирования
 */
void initrd_sefs(size_t irdst){
    if (initRD){
        return;
    }

    qemu_log("[InitRD] [SEFS] Initialization of the virtual disk. The SEFS virtual file system is used.");
    qemu_log("[InitRD] [SEFS] The virtual disk space is located at address %x.", irdst);
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
    
    char* mod_cmd[16];

    if (mods_count > 0){
		mod_start = (uint32_t*) kmalloc(sizeof(uint32_t)*mods_count);
		mod_end = (uint32_t*) kmalloc(sizeof(uint32_t)*mods_count);

        qemu_log("[kModules] Found '%d' modules",mods_count);

		for (size_t i = 0; i < mods_count; i++){
			mod_start[i] = *(uint32_t*)(multiboot->mods_addr + 8*i);
			mod_end[i] = *(uint32_t*)(multiboot->mods_addr + 8*i + 4);

            multiboot_module_t *mod = (multiboot_module_t *) (uint32_t*)(multiboot->mods_addr + 8*i);
            
            mod_cmd[i] = kcalloc(strlen((char*)mod->cmdline) + 1, sizeof(char));
            strcpy(mod_cmd[i], (char*)mod->cmdline);
            
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
 * @param multiboot_header_t mboot - Информация MultiBoot
 * @param uint32_t initial_esp -  Точка входа
 */
int kernel(multiboot_header_t* mboot, uint32_t initial_esp){
    __com_setInit(1,1);
    multiboot = mboot;
    
    drawASCIILogo(0);
    qemu_log("SayoriOS v%d.%d.%d\nBuilt: %s",
        VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH,    // Версия ядра
        __TIMESTAMP__                                   // Время окончания компиляции ядра
    );

    kHandlerCMD((char*)mboot->cmdline);
    qemu_log("Setting `Interrupt Descriptor Table`...");
    init_descriptor_tables();
    qemu_log("Setting `RIH`...");
    isr_init();
    qemu_log("Checking RAM...");
    check_memory_map((memory_map_entry_t*) mboot->mmap_addr, mboot->mmap_length);
    qemu_log("Memory manager initialization...");
    init_memory_manager(initial_esp);
    
	fpu_init();
    kModules_Init();
    init_timer(BASE_FREQ);
    asm volatile ("sti");

    qemu_log("NatSuki loading...");
    vfs_reg(PORT_COM2,VFS_TYPE_MOUNT_NATSUKI);

    text_init("/var/fonts/psf/UniCyrX-ibm-8x16.psf");
    
    qemu_log("Initializing the virtual video memory manager...");
    init_vbe(mboot);

    ata_init();
    ac97_init();

    /// MAKE COMMON INTERFACE FOR ALL 4 DISKS

    // LucarioDescriptor_t* lucario_ata00 = lucario_fs_build_descriptor();

    // lucario_fs_init(lucario_ata00, DRIVE(ATA_PRIMARY, ATA_MASTER));

    // LucarioFileEntry_t* first_entry = kcalloc(1, sizeof *first_entry);
    // lucario_fs_read_file_entry(lucario_ata00, 0, first_entry);

    // qemu_log("Info about first entry:");
    // qemu_log("\t Type: %d", first_entry->type);
    // qemu_log("\t Name: %s", first_entry->name);
    // qemu_log("\t Folder ID: %d", first_entry->folder_id);
    // qemu_log("\t Sector list at sector: %d", first_entry->sector_list_lba);
    // qemu_log("\t Sector list size (in entries): %d", first_entry->sector_list_size);
    // qemu_log("\t Size: %d", first_entry->file_size);

    // kfree(first_entry);

    // char* buf = kcalloc(64, 1);

    // lucario_fs_read_file(
    //     lucario_ata00,
    //     "test.txt",
    //     0,
    //     0,
    //     lucario_fs_file_size(
    //         lucario_ata00,
    //         "test.txt",
    //         0
    //     ),
    //     buf
    // );
    
    // qemu_log("Contents: %s", buf);

    // kfree(buf);

    // lucario_fs_destroy_descriptor(lucario_ata00);

    ///
    
    qemu_log("Initalizing fonts...");
    tty_fontConfigurate();

    tty_set_bgcolor(0x00ff00);
    tty_setcolor(0xff0000);

    // tty_printf("Hello, |%10u|%-10u|%u|\n", (unsigned int)1234567, (unsigned int)1234567, (unsigned int)1234567);

    // while(1){}

    bootScreenInit(7);
    bootScreenLazy(true);

    if (test_pcs){
        bootScreenPaint("Тестирование пищалки...");
        beeperInit(0);
    }

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
    bootScreenClose(0x000000,0xFFFFFF);
    tty_set_bgcolor(COLOR_BG);
    tty_printf("SayoriOS v%d.%d.%d\nДата компиляции: %s\n",
        VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH,    // Версия ядра
        __TIMESTAMP__                                   // Время окончания компиляции ядра
    );
    
    tty_printf("\nВлюбиться можно в красоту, но полюбить - лишь только душу.\n(c) Уильям Шекспир\n");
    
    if (__milla_getCode() != 0){
        tty_setcolor(COLOR_ERROR);
        tty_printf("[ОШИБКА] [NatSuki] Не удалось выполнить инициализацию. Код ошибки: %d",__milla_getCode());
        tty_setcolor(0xFFFFFF);
    }
    
    sayori_time_t time = get_time();
    
    tty_printf("\nВремя: %d:%d:%d\n", time.hours, time.minutes, time.seconds);
    
    keyboardInit();
    mouse_install();

    // *((uint32_t*)0xa9aba894) = 12345;

    tty_taskInit();
    qemu_log("Initialized cursor animation...");

    if (autoexec){
        // Данное условие сработает, если указан параметр ядра exec
        FILE* elf_auto = fopen(cmd_autoexec, "r");
        if (ferror(elf_auto) != 0){
            qemu_log("Autorun: Программа `%s` не найдена.\n",cmd_autoexec);
        } else {
            run_elf_file(cmd_autoexec, 0, 0);
        }
    }

    if(run_gui_at_startup)
        parallel_desktop_start();

	// TEST ZONE

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
	
    pci_print_list();
    // ac97_test();

    machinist_server_init();

    set_cursor_enabled(false);
    cpp_test();
    set_cursor_enabled(true);

    ata_list();

    /* {
        size_t size = 2 << 20;  // 1 MB
        
        uint8_t* data1 = kmalloc(size); 
        uint8_t* data2 = kmalloc(size);

        for(int i = 0; i < 5; i++) {
            size_t start = getTicks();
            memcpy2(data1, data2, size);
            size_t end = getTicks();

            qemu_log("Test #%d. Copy %d KB with memcpy() took: %d ms", i + 1, size / 1024, (end - start) / (getFrequency()/1000));
        }
    } */

    /*
    {
        size_t bufsize = (1 * 1024) * 1024;
        size_t counter = 0;

        char* data = kcalloc(bufsize, sizeof(char));

        while(1) {
            qemu_log("Read");
            ata_read(DRIVE(ATA_PRIMARY, ATA_MASTER), data, counter, bufsize);
            qemu_log("OK");

            qemu_log("Copy");
            size_t page_count = ac97_copy_user_memory_to_dma(data, bufsize);

            ac97_set_master_volume(2, 2, false);
            ac97_set_pcm_volume(2, 2, false);

            for(ssize_t i = page_count; i > 0; i-= 32) {
                ac97_single_page_write_wait(i);
            }
            qemu_log("OK");

            counter += bufsize;
    
            ac97_reset_channel();
        }
        
        ac97_reset_channel();

        kfree(data);
        ac97_destroy_user_buffer();
    }
    */

    shell();
    return 0;
}
