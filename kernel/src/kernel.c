/**
 * @file kernel.c
 * @author Пиминов Никита (nikita.piminoff@yandex.ru), NDRAEY >_ (pikachu_andrey@vk.com)
 * @brief Основная точка входа в ядро
 * @version 0.3.3
 * @date 2022-11-01
 * @copyright Copyright SayoriOS Team (c) 2022-2023
 */

#include "kernel.h"
#include "sys/sse.h"
#include "gui/parallel_desktop.h"
#include "sys/msr.h"
#include "sys/cpuinfo.h"
#include "sys/cputemp.h"
#include "drv/audio/ac97.h"
#include "sys/cpu_intel.h"
#include "sys/cpuid.h"
// For SSE
// #include <immintrin.h>

multiboot_header_t* multiboot;
uint32_t init_esp = 0;
bool initRD = false;
bool autoexec = false;
bool run_gui_at_startup = false;
char* cmd_autoexec = "";
bool test_ac97 = true;
bool test_pcs = true;
bool test_floppy = true;
bool test_network = true;
size_t kernel_start_time = 0;

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
            } else if (strcmpn(out_data[1],"floppy")){
                test_floppy = false;
                qemu_log("\t FLOPPY DISABLED");
            } else if (strcmpn(out_data[1],"network")){
                test_network = false;
                qemu_log("\t NETWORK DISABLED");
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
void initrd_sefs(size_t irdst, size_t irded){
    if (initRD){
        return;
    }

    qemu_log("[InitRD] [SEFS] Initialization of the virtual disk. The SEFS virtual file system is used.");
    qemu_log("[InitRD] [SEFS] The virtual disk space is located at address %x.", irdst);
    qemu_log("[InitRD] [SEFS] The virtual disk space is ends at %x.", irded);
    
    vfs_reg(irdst, irded, VFS_TYPE_MOUNT_SEFS);
    
    initRD = true;
}

int get_cpu_mode() {
    unsigned int cr0, cr4;

    __asm__ volatile("mov %%cr0, %0" : "=r"(cr0));
    __asm__ volatile("mov %%cr4, %0" : "=r"(cr4));

    if ((cr4 & (1 << 21)) != 0) { // Bit 21 of CR4 is set, indicating long mode
        return 64;
    } else if ((cr0 & 1) != 0 && (cr4 & (1 << 21)) == 0) { // Bit 0 of CR0 is set and bit 21 of CR4 is clear, indicating protected mode
        return 32;
    } else {
        return -1; // Unknown mode
    }
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
                initrd_sefs(mod_start[i], mod_end[i]);
                continue;
            }
		}
	} else {
        qemu_log("[kModules] No modules were connected to this operating system.");
    }
}

#ifndef RELEASE
void draw_raw_fb(multiboot_header_t* mboot, int x, int y, int w, int h, int color) {
    for(uint32_t i = y; i < y + h; i++) {
        for(uint32_t j = x; j < x + w; j++) {
            uint8_t* a = (framebuffer_addr + (j * ((((svga_mode_info_t*)mboot->vbe_mode_info)->bpp) >> 3)) + i * (((svga_mode_info_t*)mboot->vbe_mode_info)->pitch));

            a[2] = (color >> 16) & 0xff;
            a[1] = (color >> 8) & 0xff;
            a[0] = (color) & 0xff;
        }
    }
}
#endif

/**
 * @brief Точка входа в ядро
 *
 * @param multiboot_header_t mboot - Информация MultiBoot
 * @param uint32_t initial_esp -  Точка входа
 */

extern size_t CODE_start;
extern size_t CODE_end;
extern size_t DATA_start;
extern size_t DATA_end;
extern size_t RODATA_start;
extern size_t RODATA_end;
extern size_t BSS_start;
extern size_t BSS_end;
extern size_t USER_start;
extern size_t USER_end;

__attribute__((section(".user"), aligned(4096))) void user_mode() {
	uint32_t a = 0, b = 4;
	a += 1;

	uint32_t c = a + b;
	
	while(1);
}

/*
Спаси да сохрани этот кусок кода
Да на все твое кодерская воля
Да прибудет с тобой, священный код
Я тебя благославляю
*/
int kernel(multiboot_header_t* mboot, uint32_t initial_esp){
    __com_setInit(1,1);
    multiboot = mboot;

    framebuffer_addr = (uint8_t*)(mboot->framebuffer_addr);

    #ifndef RELEASE
    draw_raw_fb(mboot, 0, 0, 200, 16, 0x444444);
    #endif


    drawASCIILogo(0);
    qemu_log("SayoriOS v%d.%d.%d\nBuilt: %s",
        VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH,    // Версия ядра
        __TIMESTAMP__                                   // Время окончания компиляции ядра
    );

	qemu_log("Bootloader header at: %x", mboot);

    qemu_log("SSE check: %d", sse_check());

    if(sse_check()) {
        __wtf_fxsave();

        qemu_log("fxsave'd");

        size_t edx, unused;

        __asm__("cpuid": "=a" (unused), \
                         "=b" (unused), \
                         "=c" (unused), \
                         "=d" (edx) : \
                         "a" (1));
    
        if(edx & (1 << 26)) {
            qemu_log("Supports SSE2!");
        } else {
            #if USE_SSE
            qemu_log("WARNING: SSE2 NEEDS TO BE SUPPORTED BY YOUR PC.");
            #endif
        }
    }

    qemu_log("Current CPU mode: %d", get_cpu_mode());

    kHandlerCMD((char*)mboot->cmdline);
    qemu_log("Setting `Interrupt Descriptor Table`...");
    init_descriptor_tables();
    qemu_log("Setting `RIH`...");
    isr_init();
    qemu_log("Initializing FPU...");
	fpu_init();

    #ifndef RELEASE
    draw_raw_fb(mboot, 0, 0, 400, 16, 0x888888);
    #endif

    init_timer(BASE_FREQ);
    __asm__ volatile("sti");

    #ifndef RELEASE
    draw_raw_fb(mboot, 0, 0, 800, 16, 0xffffff);
    #endif

    qemu_log("Checking RAM...");
    check_memory_map((memory_map_entry_t*)mboot->mmap_addr, mboot->mmap_length);

	qemu_log("Memory summary:");
	qemu_log("    Code: %x - %x", &CODE_start, &CODE_end);
	qemu_log("    Data: %x - %x", &DATA_start, &DATA_end);
	qemu_log("    Read-only data: %x - %x", &RODATA_start, &RODATA_end);
	qemu_log("    BSS: %x - %x", &BSS_start, &BSS_end);
    qemu_log("Memory manager initialization...");
    init_memory_manager(initial_esp);

	// TODO: Read-only memory for .rodata segment
//	size_t rostart = &RODATA_start;
//	size_t roend = &RODATA_end;
//
//	map_pages(
//		get_kernel_dir(),
//		rostart,
//		rostart,
//		(ALIGN(roend, PAGE_SIZE) - rostart) / PAGE_SIZE,
//		PAGE_PRESENT
//	);

    kModules_Init();

    qemu_log("NatSuki loading...");
    vfs_reg(PORT_COM2, 0, VFS_TYPE_MOUNT_NATSUKI);

    text_init("/Sayori/Fonts/UniCyrX-ibm-8x16.psf");
    
    qemu_log("Initializing the virtual video memory manager...");
    init_vbe(mboot);

    clean_screen();

    qemu_log("Initalizing fonts...");
    tty_fontConfigurate();

    draw_vga_str("Initializing devices...", 23, 0, 0, 0xffffff);
    punch();

    keyboardInit();
    mouse_install();

    ata_init();
    ac97_init();

    // TESTING ZONE
    // Use this zone to enter early SayoriOS console.
    
    // while(1){}

    // END TESTING ZONE

	cputemp_calibrate();

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
    init_syscalls();

    kernel_start_time = getTicks();

    bootScreenPaint("Настройка ENV...");
    qemu_log("Registering ENV...");
    confidEnv();

    bootScreenPaint("Определение процессора...");
    detect_cpu(1);

	bootScreenPaint("Конфигурация триггеров...");
	triggersConfig();

    netcards_list_init();
    arp_init();

    rtl8139_init();

	drv_vbe_init(mboot);

    bootScreenPaint("Готово...");
    bootScreenClose(0x000000,0xFFFFFF);
    tty_set_bgcolor(COLOR_BG);

	//tga_info("/Temp/wall1.tga"); tga_paint("/Temp/wall1.tga"); while(1){}

    tty_printf("SayoriOS v%d.%d.%d\nДата компиляции: %s\n",
        VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH,    // Версия ядра
        __TIMESTAMP__                                   // Время окончания компиляции ядра
    );
    
    tty_printf("\nВлюбиться можно в красоту, но полюбить - лишь только душу.\n(c) Уильям Шекспир\n");
    
    if (__milla_getCode() != 0){
        tty_error("[ОШИБКА] [NatSuki] Не удалось выполнить инициализацию. Код ошибки: %d",__milla_getCode());
    }
    
    sayori_time_t time = get_time();
    
    tty_printf("\nВремя: %d:%d:%d\n", time.hours, time.minutes, time.seconds);

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

    _tty_printf("Listing ATA disks:\n");
    ata_list();

    // ata_dma_init();
    // ata_dma_test();

    RSDPDescriptor* rsdp = rsdp_find();

	find_facp(rsdp->RSDTaddress);

	tty_printf("APIC disabled (Unstable)\n");
    // find_apic(rsdp->RSDTaddress);
	
	if (test_network){
		_tty_printf("Listing network cards:\n");
	
		uint8_t mac_buffer[6] = {0};
	
		for(int i = 0; i < netcards_get_count(); i++) {
			netcard_entry_t* entry = netcard_get(i);
	
			_tty_printf("\tName: %s\n", entry->name);
			entry->get_mac_addr(mac_buffer);
	
			_tty_printf("\tMAC address: %v:%v:%v:%v:%v:%v\n",
						mac_buffer[0],
						mac_buffer[1],
						mac_buffer[2],
						mac_buffer[3],
						mac_buffer[4],
						mac_buffer[5]
			);
		}
	
	}
	qemu_log("Kernel bootup time: %f seconds.", (double)(getTicks() - kernel_start_time) / getFrequency());
    // NETWORK TEST

    #if 0

    netcard_entry_t* mycard = netcard_get(0);

    uint8_t destmac[6] = {0, 0, 0, 0, 0, 0};

    tty_printf("Sending packet\n");
    tty_printf("Packet sent!\n");

    ethernet_send_packet(
        mycard,
        destmac,
        "ABRACADABRA",
        11,
        0
    );

    #endif

    // NETWORK TEST END

	if (test_floppy){
		initFloppy();
		fatTest();
		_smfs_init();
	}

	tty_printf("Processors found: %d\n", system_processors_found);
	// _mbr_info();

    // ata_dma_init();
    // ata_dma_test();

    //////////////////

	// TODO: User mode!

    // scheduler_mode(false);

    // tty_printf("Starting v8086\n");
    // v8086_enable();
    // tty_printf("Enabled?\n");


	// Tried testing user mode

	// qemu_log("Start: %x", &USER_start);
	// qemu_log("End: %x", &USER_end);
// 
	// map_pages(
		// get_kernel_dir(),
		// (virtual_addr_t) &USER_start,
		// ALIGN((physaddr_t) &USER_end, PAGE_SIZE),
		// 1,
		// PAGE_USER | PAGE_WRITEABLE | PAGE_PRESENT
	// );
// 
	// qemu_log("Mapped");
// 
	// scheduler_mode(false);
// 
	// size_t addr = alloc_phys_pages(16);
	// map_pages(
		// get_kernel_dir(),
		// addr,
		// addr,
		// 16,
		// PAGE_USER | PAGE_WRITEABLE | PAGE_PRESENT
	// );
// 
	// qemu_log("Stack: %x", addr);
// 
	// user_mode_switch(user_mode, addr + (16 * PAGE_SIZE));
// 
	// qemu_printf("WHAT");

    shell();
    return 0;
}
