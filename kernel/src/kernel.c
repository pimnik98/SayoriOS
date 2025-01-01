/**
 * @file kernel.c
 * @author Пиминов Никита (nikita.piminoff@yandex.ru), NDRAEY >_ (pikachu_andrey@vk.com)
 * @brief Основная точка входа в ядро
 * @version 0.3.5
 * @date 2022-11-01
 * @copyright Copyright SayoriOS Team (c) 2022-2024
 */

#include "kernel.h"

#include <drv/fpu.h>
#include <lib/php/explode.h>
#include <sys/unwind.h>

#include "mem/pmm.h"
#include "mem/vmm.h"
#include "drv/audio/ac97.h"
#include "sys/mtrr.h"
#include "net/ipv4.h"

#include "fs/natfs.h"
#include "net/stack.h"
#include "drv/audio/hda.h"
#include "lib/ttf_font.h"
#include "sys/grub_modules.h"
#include "drv/disk/mbr.h"
#include "sys/file_descriptors.h"
#include "sys/lapic.h"
#include "drv/ps2.h"
#include "net/dhcp.h"
#include "gfx/intel.h"
#include "ports/eBat/eBat.h"
#include "ports/eBat/eBatRuntime.h"


#include <lib/pixel.h>

#define INITRD_RW_SIZE (1474560) ///< Размер виртуального диска 1.44mb floppy

extern bool ps2_channel2_okay;

uint32_t init_esp = 0;
bool test_pcs = true;
bool test_floppy = true;
bool test_network = true;
bool is_rsdp = true;
bool initRD = false;
size_t kernel_start_time = 0;
size_t ramdisk_size = INITRD_RW_SIZE;

void jse_file_getBuff(char* buf);
void kHandlerCMD(char*);

void autoexec(){

    variable_write("HOSTNAME", "SAYORISOUL");
    variable_write("SYSTEMROOT", "R:\\Sayori\\");
    variable_write("TEMP", "T:\\");
    variable_write("USERNAME", "OEM");
    variable_write("BUILDUSER", BUILDUSER);
    variable_write("BUILDDATA", __TIMESTAMP__);
    variable_write("VERSION_MAJOR", TOSTRING(VERSION_MAJOR));
    variable_write("VERSION_MINOR", TOSTRING(VERSION_MINOR));
    variable_write("VERSION_PATCH", TOSTRING(VERSION_PATCH));
    variable_write("ARCH_TYPE", ARCH_TYPE);
    variable_write("VERNAME", VERNAME);
    variable_write("SUBVERSIONNAME", SUBVERSIONNAME);
    variable_write("VERSION", VERSION_STRING);


    char* f = "R:\\autoexec.bat";
    FILE* cat_file = fopen(f, "r");
    if (!cat_file){
        tty_setcolor(COLOR_ERROR);
        tty_printf("[AutoExec] Не удалось найти файл `%s`.\n",f);
        return;
    }

    size_t filesize = fsize(cat_file);

    uint8_t* buffer = kcalloc(1,filesize + 1);

    fread(cat_file, 1, filesize, buffer);

    qemu_log("'%s'", buffer);

    BAT_T* token = bat_parse_string(buffer);
    token->Debug = 0;
    token->Echo = 1;
    int ret = bat_runtime_exec(token);
    qemu_warn("RETURN CODE: %d\n",ret);
    bat_destroy(token);


    fclose(cat_file);

    kfree(buffer);
}

void __createRamDisk(){
    qemu_note("[INITRD] Create virtual read-write disk...");
    void* disk_t = kmalloc(INITRD_RW_SIZE+1);
    if (disk_t == NULL){
        qemu_err("[INITRD] Fatal create virtual disk");
        return;
    }
    qemu_log("[INITRD] Temp disk is (%d bytes) created to %x", ramdisk_size, disk_t);
    dpm_reg('T',"TempDisk","TEMPFS", 2, ramdisk_size, 0, 0, 2, "TEMP-DISK", disk_t);
    fs_tempfs_format('T');
    qemu_ok("[INITRD] The virtual hard disk has been successfully created.");
}

#ifndef RELEASE
void draw_raw_fb(multiboot_header_t* mboot, int x, int y, int w, int h, int color) {
    for(uint32_t i = y; i < y + h; i++) {
        for(uint32_t j = x; j < x + w; j++) {
            uint8_t* a = (framebuffer_addr + (j * ((mboot->framebuffer_bpp) >> 3)) + i * (mboot->framebuffer_pitch));

            a[2] = (color >> 16) & 0xff;
            a[1] = (color >> 8) & 0xff;
            a[0] = (color) & 0xff;
        }
    }
}
#else
#define draw_raw_fb(a, b, c, d, e, f)
#endif

/**
 * @brief Обработка команд указаных ядру при загрузке
 *
 * @param cmd - Команды
 */

void kHandlerCMD(char* cmd){
    qemu_log("Kernel command line at address %x and contains: '%s'", (size_t)cmd, cmd);

    if(strlen(cmd) == 0)
        return;

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
            } else if (strcmpn(out_data[1],"dark")) {
                bootScreenChangeTheme(0);
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
        if (strcmpn(out_data[0],"ramdisk")){
            ramdisk_size = atoi(out_data[1]);
        }
        if (strcmpn(out_data[0],"disable")){
            if (strcmpn(out_data[1],"coms")){
                // FIXME: If uncomment following line of code, it willn't boot
                __com_setInit(1, 0);
                __com_setInit(2, 0);
                __com_setInit(3, 0);
                __com_setInit(4, 0);
                qemu_log("\t COM-OUT DISABLED");
            } else if (strcmpn(out_data[1],"floppy")){
                test_floppy = false;
                qemu_log("\t FLOPPY DISABLED");
            } else if (strcmpn(out_data[1],"network")){
                test_network = false;
                qemu_log("\t NETWORK DISABLED");
            } else if (strcmpn(out_data[1],"pc-speaker")){
                test_pcs = false;
                qemu_log("\t PC-Speaker DISABLED");
            } else if (strcmpn(out_data[1],"rdsp")){
                is_rsdp = false;
                qemu_log("\t RDSP DISABLED");
            }  else {
                qemu_log("\t Sorry, no support!");
            }
        }
        //qemu_log("[kCMD] [%d] %s >\n\tKey: %s\n\tValue:%s",i,out[i],out_data[0],out_data[1]);
    }
}

/**
 * @brief Монтирует виртуальный диск с файловой системой Sayori Easy File System
 *
 * @param irdst - Точка монтирования
 * @param irded - Конец точки монтирования
 */

void initrd_sefs(size_t irdst, size_t irded){
    if (initRD){
        return;
    }

    qemu_log("[InitRD] [SEFS] Initialization of the virtual disk. The SEFS virtual file system is used.");
    qemu_log("[InitRD] [SEFS] The virtual disk space is located at address %x.", irdst);
    qemu_log("[InitRD] [SEFS] The virtual disk space is ends at %x.", irded);
}

/**
 * @brief Точка входа в ядро
 *
 * @param multiboot_header_t mboot - Информация MultiBoot
 * @param initial_esp -  Точка входа
 */

extern size_t CODE_start;
extern size_t CODE_end;
extern size_t DATA_start;
extern size_t DATA_end;
extern size_t RODATA_start;
extern size_t RODATA_end;
extern size_t BSS_start;
extern size_t BSS_end;

/*
  Спаси да сохрани этот кусок кода
  Да на все твое кодерская воля
  Да прибудет с тобой, священный код
  Я тебя благославляю
*/
void  __attribute__((noreturn)) kmain(multiboot_header_t* mboot, uint32_t initial_esp) {
    __com_setInit(1, 1);
    __com_init(PORT_COM1);
    
    __asm__ volatile("movl %%esp, %0" : "=r"(init_esp));
    
    framebuffer_addr = (uint8_t *) (mboot->framebuffer_addr);
    
    draw_raw_fb(mboot, 0, 0, 200, 16, 0x444444);
    
    drawASCIILogo(0);
    
    qemu_log("SayoriOS v%d.%d.%d\nBuilt: %s",
             VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH,    // Версия ядра
             __TIMESTAMP__                                   // Время окончания компиляции ядра
        );
    
    qemu_log("Bootloader header at: %x", (size_t)mboot);
    
    qemu_log("SSE: %s", sse_check() ? "Supported" : "Not supported");
    
    if (sse_check()) {
        fpu_save();
    }
    
    qemu_log("Setting `Interrupt Descriptor Table`...");
    init_descriptor_tables();
    qemu_log("Setting `RIH`...");
    isr_init();
    
    qemu_log("Initializing FPU...");
    fpu_init();
    
    draw_raw_fb(mboot, 0, 0, 400, 16, 0x888888);
    
    init_timer(CLOCK_FREQ);
    
    __asm__ volatile("sti");
    
    draw_raw_fb(mboot, 0, 0, 800, 16, 0xffffff);
    
    qemu_log("Checking RAM...");
    check_memory_map((memory_map_entry_t *) mboot->mmap_addr, mboot->mmap_length);
    qemu_log("Memory summary:");
    qemu_log("    Code: %x - %x", (size_t)&CODE_start, (size_t)&CODE_end);
    qemu_log("    Data: %x - %x", (size_t)&DATA_start, (size_t)&DATA_end);
    qemu_log("    Read-only data: %x - %x", (size_t)&RODATA_start, (size_t)&RODATA_end);
    qemu_log("    BSS: %x - %x", (size_t)&BSS_start, (size_t)&BSS_end);
    qemu_log("Memory manager initialization...");
    
    grub_modules_prescan(mboot);
    
    init_paging();
    
    mark_reserved_memory_as_used((memory_map_entry_t *) mboot->mmap_addr, mboot->mmap_length);
    
    qemu_ok("PMM Ok!");
    
    vmm_init();
    qemu_ok("VMM OK!");
    
    switch_qemu_logging();
    
    kHandlerCMD((char *) mboot->cmdline);
    
    drv_vbe_init(mboot);

    qemu_log("Registration of file system drivers...");
    fsm_reg("TARFS", 1, &fs_tarfs_read, &fs_tarfs_write, &fs_tarfs_info, &fs_tarfs_create, &fs_tarfs_delete,
            &fs_tarfs_dir, &fs_tarfs_label, &fs_tarfs_detect);
    fsm_reg("FAT32", 1, &fs_fat32_read, &fs_fat32_write, &fs_fat32_info, &fs_fat32_create, &fs_fat32_delete,
            &fs_fat32_dir, &fs_fat32_label, &fs_fat32_detect);
    fsm_reg("NatFS", 1, &fs_natfs_read, &fs_natfs_write, &fs_natfs_info, &fs_natfs_create, &fs_natfs_delete,
            &fs_natfs_dir, &fs_natfs_label, &fs_natfs_detect);
    fsm_reg("ISO9660", 1, &fs_iso9660_read, &fs_iso9660_write, &fs_iso9660_info, &fs_iso9660_create, &fs_iso9660_delete,
            &fs_iso9660_dir, &fs_iso9660_label, &fs_iso9660_detect);
    fsm_reg("TEMPFS", 1, &fs_tempfs_read, &fs_tempfs_write, &fs_tempfs_info, &fs_tempfs_create, &fs_tempfs_delete,
            &fs_tempfs_dir, &fs_tempfs_label, &fs_tempfs_detect);
    fs_natfs_init();

    grub_modules_init(mboot);
    
    kernel_start_time = getTicks();

    mtrr_init();
    text_init("R:\\Sayori\\Fonts\\UniCyrX-ibm-8x16.psf");
    // /Sayori/Fonts/UniCyrX-ibm-8x16.psf
    
    qemu_log("Initializing the virtual video memory manager...");
    init_vbe(mboot);
    
    qemu_log("Initializing Task Manager...");
    init_task_manager();
    
    clean_screen();
    
    qemu_log("Initalizing fonts...");
    tty_fontConfigurate();
    
    draw_vga_str("Initializing devices...", 23, 0, 0, 0xffffff);
    punch();

    bootScreenInit(15);
    bootScreenLazy(true);

    bootScreenPaint("Настройка PS/2...");
    ps2_init();
    bootScreenPaint("Настройка PS/2 Клавиатуры...");
    keyboardInit();

	if(ps2_channel2_okay) {
	    bootScreenPaint("Настройка PS/2 Мыши...");
	    mouse_install();		
	}

    bootScreenPaint("Пост-настройка PS/2...");
    ps2_keyboard_install_irq();
    ps2_mouse_install_irq();

    bootScreenPaint("PCI Setup...");
    pci_scan_everything();

    bootScreenPaint("Инициализация ATA...");
    ata_init();
    ata_dma_init();

    bootScreenPaint("Калибровка датчика температуры процессора...");
    cputemp_calibrate();

    bootScreenPaint("Настройка FDT...");
    file_descriptors_init();

    char* btitle = 0;

    asprintf(&btitle, "Создание виртуального диска (%u kb.)...", ramdisk_size/1024);

    bootScreenPaint(btitle);
    kfree(btitle);
    __createRamDisk();
    
    bootScreenPaint("Настройка системных вызовов...");
    qemu_log("Registering System Calls...");
    init_syscalls();
    
    bootScreenPaint("Настройка ENV...");
    qemu_log("Registering ENV...");
    configure_env();
    
    bootScreenPaint("Определение процессора...");
    detect_cpu(1);
    
    bootScreenPaint("Конфигурация триггеров...");
    triggersConfig();
    
    bootScreenPaint("Инициализация списка сетевых карт...");
    netcards_list_init();
    
    bootScreenPaint("Инициализация сетевого стека...");
    netstack_init();
    bootScreenPaint("Инициализация ARP...");
    arp_init();
    bootScreenPaint("Инициализация RTL8139...");
    rtl8139_init();
    bootScreenPaint("Инициализация DHCP...");
    dhcp_init_all_cards();
    bootScreenPaint("Готово...");
    bootScreenClose(0x000000, 0xFFFFFF);
    tty_set_bgcolor(COLOR_BG);
    
    tty_printf("SayoriOS v%d.%d.%d\nДата компиляции: %s\n",
               VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH,    // Версия ядра
               __TIMESTAMP__                                   // Время окончания компиляции ядра
        );
    tty_printf("\nВлюбиться можно в красоту, но полюбить - лишь только душу.\n(c) Уильям Шекспир\n");
    
    if (__milla_getCode() != 0) {
        tty_error("[ОШИБКА] [NatSuki] Не удалось выполнить инициализацию. Код ошибки: %d", __milla_getCode());
    }
    
    sayori_time_t time = get_time();
    tty_printf("\nВремя: %d:%d:%d\n", time.hours, time.minutes, time.seconds);
    
    _tty_printf("Listing ATA disks:\n");
    ata_list();
    
    tty_taskInit();
    
    if (is_rsdp){
        RSDPDescriptor* rsdp = rsdp_find();
        qemu_log("RSDP at: %x", rsdp);

        if(rsdp) {
			acpi_scan_all_tables(rsdp->RSDTaddress);
			
	        find_facp(rsdp->RSDTaddress);

            lapic_init(rsdp);
        } else {
            tty_printf("ACPI not supported! (Are you running in UEFI mode?)\n");
            qemu_err("ACPI not supported! (Are you running in UEFI mode?)");
        }
    }
    
    tty_printf("Processors: %d\n", system_processors_found);

    // FIXME: WOW! We can write into 0!
    //	*((volatile int*)0) = 0x12345678;
    //	qemu_log("Data: %x", *((volatile int*)0));
    
    if (test_network) {
        _tty_printf("Listing network cards:\n");
    
        uint8_t mac_buffer[6] = {0};
    
        for (int i = 0; i < netcards_get_count(); i++) {
            netcard_entry_t *entry = netcard_get(i);
    
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

    //	if (test_floppy){
    //		initFloppy();
    //		fatTest();
    //		_smfs_init();
    //	}
    
    ac97_init();
    
    /// Пример закругленных квадратов
    // drawRoundedSquare(32,32, 128, 2, 0xFFFF0000, 0xFF0000FF);
    // drawRoundedRectangle(32,32,128,16,4,0xFFFF0000, 0xFF0000FF);
    // punch();
    // while (1){}
    /// КОНЕЦ ПРИМЕРА

    ahci_init();

    /// Обновим данные обо всех дисках
    fsm_dpm_update(-1);    
    
    // vio_ntw_init();

// 	size_t hwstart = timestamp();
// 
// 	for(int i = 0, sh = getScreenHeight(); i < sh; i+=20) {	
// 		for(int j = 0, sw = getScreenWidth(); j < sw; j+=20) {
// 			draw_filled_rectangle(j, i, 20, 20, rand());
// 		}
// 	}
// 
// 	qemu_note("Program finished generating rects in %d ms", hwstart);
// 
// 	punch();
// 
// 	while(1);

    igfx_init();

//    hda_init();
	// void k();

//	 create_process(k, "process", false, true);
//    sleep_ms(500);
//    create_process(k, "process2", false, true);
//    sleep_ms(1500);
//    create_process(k, "process3", false, true);

    autoexec();

    qemu_log("System initialized everything at: %f seconds.", (double) (getTicks() - kernel_start_time) / getFrequency());

    // char* args[] = {};
    // spawn("R:\\hellors", 0, args);

    cli();

    while(1)
        ;
}

// void k() {
//     for(int i = 0; i < 10; i++) {
//         qemu_err("HELLO");
//         sleep_ms(250);
//     }
// }
