/**
 * @file kernel.c
 * @author Пиминов Никита (nikita.piminoff@yandex.ru), NDRAEY >_ (pikachu_andrey@vk.com)
 * @brief Основная точка входа в ядро
 * @version 0.3.4
 * @date 2022-11-01
 * @copyright Copyright SayoriOS Team (c) 2022-2023
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
#include "lib/freeada/ada.h"

#include "fs/natfs.h"

#include <lib/pixel.h>

multiboot_header_t* multiboot;
uint32_t init_esp = 0;
bool initRD = false;
bool autoexec = false;
char* cmd_autoexec = "";
//bool test_ac97 = true;
bool test_pcs = true;
bool test_floppy = true;
bool test_network = true;
bool is_rsdp = true;
size_t kernel_start_time = 0;

void jse_file_getBuff(char* buf);

/**
 * @brief Обработка комманд указаных ядру при загрузке
 *
 * @param cmd - Команды
 */

void kHandlerCMD(char* cmd){
    qemu_log("Kernel command line at address %x and contains: '%s'", cmd, cmd);

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
 * @brief Инициализирует модули подключенные к ОС
 *
 */

size_t last_module_end = 0;

void scan_kmodules() {
	uint32_t	mods_count = multiboot->mods_count;

	for (size_t i = 0; i < mods_count; i++){
		multiboot_module_t *mod = (multiboot_module_t *) (uint32_t*)(multiboot->mods_addr + 8*i);

		last_module_end = mod->mod_end;
	}

	qemu_log("Last module ends at: %x", last_module_end);
}

void kModules_Init(){
    qemu_log("[kModules] Loading operating system modules...");
    uint32_t	mod_start[32] = {0};
    uint32_t	mod_end[32] = {0};
    uint32_t	mod_size[32] = {0};
    uint32_t	mods_count = multiboot->mods_count;

    char mod_cmd[32][64] = {0};

    if (mods_count > 0){
        qemu_log("[kModules] Found %d modules",mods_count);

		for (size_t i = 0; i < mods_count; i++){
			mod_start[i] = *(uint32_t*)(multiboot->mods_addr + 8*i);
			mod_end[i] = *(uint32_t*)(multiboot->mods_addr + 8*i + 4);

			mod_size[i] = mod_end[i] - mod_start[i];

            multiboot_module_t *mod = (multiboot_module_t *) (uint32_t*)(multiboot->mods_addr + 8*i);
            
            strcpy(mod_cmd[i], (char*)mod->cmdline);
            
            qemu_log("[kModules] Found module number `%d`. (Start: %x | End: %x | Size: %d) CMD: %s (%s)",
					 i,
					 mod_start[i],
					 mod_end[i],
					 mod_size[i],
					 mod_cmd[i],
					 (char*)mod->cmdline
					 );

            if (strcmpn(mod_cmd[i],"initrd_sefs")){
                initrd_sefs(mod_start[i], mod_end[i]);
                continue;
            }
            if (strcmpn(mod_cmd[i],"initrd_tarfs")){
                initrd_tarfs(mod_start[i], mod_end[i]);
            }
		}

		qemu_log("Memory manager need to be feed with this information: Last module ends at: %x", last_module_end);
	} else {
        qemu_log("[kModules] No modules were connected to this operating system.");
    }
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
extern size_t USER_start;
extern size_t USER_end;

/*
Спаси да сохрани этот кусок кода
Да на все твое кодерская воля
Да прибудет с тобой, священный код
Я тебя благославляю
*/
int kernel(multiboot_header_t* mboot, uint32_t initial_esp) {
	__com_setInit(1, 1);
	multiboot = mboot;

	__asm__ volatile("movl %%esp, %0" : "=r"(init_esp));

	framebuffer_addr = (uint8_t *) (mboot->framebuffer_addr);

	draw_raw_fb(mboot, 0, 0, 200, 16, 0x444444);

	drawASCIILogo(0);

	qemu_log("SayoriOS v%d.%d.%d\nBuilt: %s",
			 VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH,    // Версия ядра
			 __TIMESTAMP__                                   // Время окончания компиляции ядра
	);

	qemu_log("Bootloader header at: %x", mboot);

	qemu_log("SSE: %s", sse_check() ? "Supported" : "Not supported");

	if (sse_check()) {
		__wtf_fxsave();
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
	qemu_log("    Code: %x - %x", &CODE_start, &CODE_end);
	qemu_log("    Data: %x - %x", &DATA_start, &DATA_end);
	qemu_log("    Read-only data: %x - %x", &RODATA_start, &RODATA_end);
	qemu_log("    BSS: %x - %x", &BSS_start, &BSS_end);
	qemu_log("Memory manager initialization...");

	scan_kmodules();

	init_paging();

	mark_reserved_memory_as_used((memory_map_entry_t *) mboot->mmap_addr, mboot->mmap_length);

	qemu_ok("PMM Ok!");

	vmm_init();

	qemu_ok("VMM OK!");

    switch_qemu_logging();

	kHandlerCMD((char *) mboot->cmdline);

	qemu_log("Registration of file system drivers...");
	fsm_reg("TARFS", 1, &fs_tarfs_read, &fs_tarfs_write, &fs_tarfs_info, &fs_tarfs_create, &fs_tarfs_delete,
			&fs_tarfs_dir, &fs_tarfs_label, &fs_tarfs_detect);
	fsm_reg("FAT32", 1, &fs_fat32_read, &fs_fat32_write, &fs_fat32_info, &fs_fat32_create, &fs_fat32_delete,
			&fs_fat32_dir, &fs_fat32_label, &fs_fat32_detect);
    fsm_reg("NatFS", 1, &fs_natfs_read, &fs_natfs_write, &fs_natfs_info, &fs_natfs_create, &fs_natfs_delete,
            &fs_natfs_dir, &fs_natfs_label, &fs_natfs_detect);
    fsm_reg("ISO9660", 1, &fs_iso9660_read, &fs_iso9660_write, &fs_iso9660_info, &fs_iso9660_create, &fs_iso9660_delete,
            &fs_iso9660_dir, &fs_iso9660_label, &fs_iso9660_detect);

    fs_natfs_init();
	kModules_Init();

	mtrr_init();

	text_init("R:\\Sayori\\Fonts\\UniCyrX-ibm-8x16.psf");

	qemu_log("Initializing the virtual video memory manager...");
	init_vbe(mboot);

	qemu_log("Initializing Task Manager...");
	init_task_manager();
	clean_screen();

	qemu_log("Initalizing fonts...");
	tty_fontConfigurate();

	draw_vga_str("Initializing devices...", 23, 0, 0, 0xffffff);
	punch();

	keyboardInit();
	mouse_install();

	ata_init();

	cputemp_calibrate();

	bootScreenInit(9);
	bootScreenLazy(true);

	bootScreenPaint("Настройка системных вызовов...");
	qemu_log("Registering System Calls...");
	init_syscalls();

	kernel_start_time = getTicks();

	bootScreenPaint("Настройка ENV...");
	qemu_log("Registering ENV...");
	configure_env();

	bootScreenPaint("Определение процессора...");
	detect_cpu(1);

	bootScreenPaint("Конфигурация триггеров...");
	triggersConfig();

    bootScreenPaint("Инициализация списка сетевых карт...");
    netcards_list_init();

    bootScreenPaint("Инициализация ARP...");
    arp_init();

    bootScreenPaint("Инициализация RTL8139...");
    rtl8139_init();

    bootScreenPaint("Инициализация DHCP...");
    dhcp_init_all_cards();

	bootScreenPaint("Готово...");
	bootScreenClose(0x000000, 0xFFFFFF);
	tty_set_bgcolor(COLOR_BG);

    drv_vbe_init(mboot);

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

        find_facp(rsdp->RSDTaddress);
        find_apic(rsdp->RSDTaddress);
    }

	tty_printf("Processors: %d\n", system_processors_found);

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
	qemu_log("Kernel bootup time: %f seconds.", (double) (getTicks() - kernel_start_time) / getFrequency());
	ata_dma_init();
    ac97_init();
	
	ahci_init();

    /// Обновим данные обо всех дисках
    fsm_dpm_update(-1);

	cli();

    return 0;
}
