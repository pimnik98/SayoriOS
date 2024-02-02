/**
 * @file sys/cpuinfo.c
 * @author Пиминов Никита (nikita.piminoff@yandex.ru)
 * @brief Определение процессора
 * @version 0.3.5
 * @date 2022-10-01
 * @copyright Copyright SayoriOS Team (c) 2022-2024
 */

#include "common.h"
#include "lib/string.h"
#include "portability.h"
#include <io/ports.h>
#include <sys/cpuinfo.h>

char brandAllName[128] = {0};                ///< Название процессора


#define tty_printf(M, ...)

#define INTEL_MAGIC                     0x756e6547      ///< Ключ процессора Intel
#define AMD_MAGIC                       0x68747541      ///< Ключ процессора AMD
#define VMWARE_HYPERVISOR_MAGIC         0x564D5868      ///< Ключ гипервизора VMWare
#define VMWARE_HYPERVISOR_PORT          0x5658          ///< Порт доступа данных к VMWare
#define VMWARE_PORT_CMD_GETVERSION      10              ///< Версия управления VWMare


#define VMWARE_PORT(cmd, eax, ebx, ecx, edx)                            \
        __asm__("inl (%%dx)" :                                          \
                        "=a"(eax), "=c"(ecx), "=d"(edx), "=b"(ebx) :    \
                        "0"(VMWARE_HYPERVISOR_MAGIC),                   \
                        "1"(VMWARE_PORT_CMD_##cmd),                     \
                        "2"(VMWARE_HYPERVISOR_PORT), "3"(UINT_MAX) :    \
                        "memory");

/**
 * @brief Получение имени процессора (Инициализация)
 *
 * @param silent - Тихий режим
 *
 * @return int - Тип процессора (0 - Unknown | 1 - Intel | 2 - AMD | 3 - VMWare)
 *
 * @warning Не для личного использования. Если вы хотите получить название процессора используйте функцию getNameBrand()
 */
int detect_cpu(bool silent) {
    int type;
    uint32_t ebx, unused;
    cpuid(0, unused, ebx, unused, unused);
    switch (ebx) {
    case INTEL_MAGIC: /* Intel Magic Code */
        //strcat(brandAllName,"Intel ");
        do_intel(silent);
        type = 1;
        break;
    case AMD_MAGIC: /* AMD Magic Code */
        //strcat(brandAllName,"AMD ");
        do_amd(silent);
        type = 2;
        break;
    case VMWARE_HYPERVISOR_MAGIC: /* VMWARE_HW_MAGIC */
        memset(brandAllName, 0, 128);
        strcat(brandAllName, "VMWARE_HYPERVISOR_MAGIC");
        type = 3;
        break;
    default:
        strcat(brandAllName,"Unknown x86");
        type = 0;
        //qemu_log("Unknown x86 CPU Detected\n");
        break;
    }
    qemu_log("[CPU] Detect: %s",brandAllName);
    return type;
}

/**
 * @brief Получение имени процессора
 *
 * @return char* - Полное имя процессора
 */
char* getNameBrand(){
    return brandAllName;
}

char *Intel[] = {
    "Brand ID Not Supported.",
    "Intel(R) Celeron(R) processor",
    "Intel(R) Pentium(R) III processor",
    "Intel(R) Pentium(R) III Xeon(R) processor",
    "Intel(R) Pentium(R) III processor",
    "Reserved",
    "Mobile Intel(R) Pentium(R) III processor-M",
    "Mobile Intel(R) Celeron(R) processor",
    "Intel(R) Pentium(R) 4 processor",
    "Intel(R) Pentium(R) 4 processor",
    "Intel(R) Celeron(R) processor",
    "Intel(R) Xeon(R) Processor",
    "Intel(R) Xeon(R) processor MP",
    "Reserved",
    "Mobile Intel(R) Pentium(R) 4 processor-M",
    "Mobile Intel(R) Pentium(R) Celeron(R) processor",
    "Reserved",
    "Mobile Genuine Intel(R) processor",
    "Intel(R) Celeron(R) M processor",
    "Mobile Intel(R) Celeron(R) processor",
    "Intel(R) Celeron(R) processor",
    "Mobile Geniune Intel(R) processor",
    "Intel(R) Pentium(R) M processor",
    "Mobile Intel(R) Celeron(R) processor"
}; ///< Лист-спецификаций Intel

/**
 * @brief - Дополнительная таблица спецификаций Intel
 *
 * @warning - Эта таблица предназначена для тех строк брендов, которые имеют два значения в зависимости от подписи процессора. В ней должно быть то же количество записей, что и в приведенной выше таблице.
 */
char *Intel_Other[] = {
    "Reserved",
    "Reserved",
    "Reserved",
    "Intel(R) Celeron(R) processor",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Intel(R) Xeon(R) processor MP",
    "Reserved",
    "Reserved",
    "Intel(R) Xeon(R) processor",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved"
};

/**
 * @brief Получение информации о процессоре Intel
 *
 * @param silent - Тихий режим
 *
 * @return int - 0
 *
 * @warning Не для личного использования. Если вы хотите получить название процессора используйте функцию getNameBrand()
 */
int do_intel(bool silent) {
    if (silent == false){
        qemu_log("Detected Intel CPU.\nIntel-specific features:\n");
    }

//    uint32_t dtc1, dtc2, dtc3, dtc4, dtc5;
    uint32_t eax, ebx, max_eax, signature, unused;
    uint32_t model, family, type, brand;//, stepping, reserved;
//    uint32_t extended_family;
    cpuid(1, eax, ebx, unused, unused);
    model = (eax >> 4) & 0xf;
    family = (eax >> 8) & 0xf;
    type = (eax >> 12) & 0x3;
    brand = ebx & 0xff;
//    stepping = eax & 0xf;
//    reserved = eax >> 14;
    signature = eax;
    if (silent == 0){
        qemu_log("Type %d - ", type);
		switch(type) {
			case 0:
                qemu_log("Original OEM");
				break;
			case 1:
                qemu_log("Overdrive");
				break;
			case 2:
                qemu_log("Dual-capable");
				break;
			case 3:
                qemu_log("Reserved");
				break;
			default:
                qemu_log("Unknown");
        }
        qemu_log("Family %d - ", family);
        switch(family) {
			case 3:
                qemu_log("i386");
				break;
			case 4:
                qemu_log("i486");
				break;
			case 5:
                qemu_log("Pentium");
				break;
			case 6:
                qemu_log("Pentium Pro");
				break;
			case 15:
                qemu_log("Pentium 4");
			default:
                qemu_log("Unknown");
        }
        if (family == 15) {
//            extended_family = (eax >> 20) & 0xff;
            qemu_log("Extended family %d\n", (eax >> 20) & 0xff);
        }
        qemu_log("Model %d - ", model);

        switch (family) {
			case 3:
				break;
			case 4:
				switch (model) {
					case 0:
					case 1:
                        qemu_log("DX");
						break;
					case 2:
                        qemu_log("SX");
						break;
					case 3:
                        qemu_log("487/DX2");
						break;
					case 4:
                        qemu_log("SL");
						break;
					case 5:
                        qemu_log("SX2");
						break;
					case 7:
                        qemu_log("Write-back enhanced DX2");
						break;
					case 8:
                        qemu_log("DX4");
						break;
					default:
						break;
				}
				break;
			case 5:
				switch (model) {
					case 1:
                        qemu_log("60/66");
						break;
					case 2:
                        qemu_log("75-200");
						break;
					case 3:
                        qemu_log("for 486 system");
						break;
					case 4:
                        qemu_log("MMX");
						break;
					default:
						break;
				}
				break;
			case 6:
				switch (model) {
					case 1:
                        qemu_log("Pentium Pro");
						break;
					case 3:
                        qemu_log("Pentium II Model 3");
						break;
					case 5:
                        qemu_log("Pentium II Model 5/Xeon/Celeron");
						break;
					case 6:
                        qemu_log("Celeron");
						break;
					case 7:
                        qemu_log("Pentium III/Pentium III Xeon - external L2 cache");
						break;
					case 8:
                        qemu_log("Pentium III/Pentium III Xeon - internal L2 cache");
						break;
					default:
						break;
				}
				break;
			default:
				break;
        }
    }
    cpuid(0x80000000, max_eax, unused, unused, unused);

    /* Quok said: If the max extended eax value is high enough to support the processor brand string
    (values 0x80000002 to 0x80000004), then we'll use that information to return the brand information.
    Otherwise, we'll refer back to the brand tables above for backwards compatibility with older processors.
    According to the Sept. 2006 Intel Arch Software Developer's Guide, if extended eax values are supported,
    then all 3 values for the processor brand string are supported, but we'll test just to make sure and be safe. */

    if (max_eax >= 0x80000004) {
        unsigned int a, b, c, d;
        int inx = 0, aa = 0;
        memset(brandAllName,0,128);
        for (int i = 0x80000002; i <= 0x80000004; ++i) {
            cpuid(i, a, b, c, d);
            // Конкатенация информации о модели в processorModel
            for (aa = 0; aa < 4; aa++){
                char* delta = (char*)&a;
                brandAllName[inx] = delta[aa];
                inx++;
            }
            for (aa = 0; aa < 4; aa++){
                char* delta = (char*)&b;
                brandAllName[inx] = delta[aa];
                inx++;
            }
            for (aa = 0; aa < 4; aa++){
                char* delta = (char*)&c;
                brandAllName[inx] = delta[aa];
                inx++;
            }
            for (aa = 0; aa < 4; aa++){
                char* delta = (char*)&d;
                brandAllName[inx] = delta[aa];
                inx++;
            }
            //qemu_log("[%s] [%s] [%s] [%s]", &a, &b, &c, &d);
        }

        //qemu_log("%s", brandAllName);
    } else if (brand > 0 && silent == 0) {
        qemu_log("Brand %d - ", brand);
        if (brand < 0x18) {
            if (signature == 0x000006B1 || signature == 0x00000F13) {
                qemu_log("%s\n", Intel_Other[brand]);
            } else {
                qemu_log("%s\n", Intel[brand]);
            }
        } else {
            qemu_log("Reserved\n");
        }
    } else {
        qemu_log("Other INTEL CPU\n");
    }
    if (silent == 0){
        //qemu_log("Stepping: %d Reserved: %d\n", stepping, reserved);
    }
    return 0;
}

/**
 * @brief Получение информации о процессоре AMD
 *
 * @param silent - Тихий режим
 *
 * @return int - 0
 *
 * @warning Не для личного использования. Если вы хотите получить название процессора используйте функцию getNameBrand()
 */
int do_amd(bool silent) {
    if (silent == 0){
        qemu_log("Detected AMD CPU. \nAMD-specific features:\n");
    }

    uint32_t extended, eax, ebx, ecx, edx, unused;
    uint32_t family, model;//, stepping, reserved;
    cpuid(1, eax, unused, unused, unused);

    model = (eax >> 4) & 0xf;
    family = (eax >> 8) & 0xf;
//    stepping = eax & 0xf;
//    reserved = eax >> 12;
    if (silent == 0){
        qemu_log("Family: %d Model: %d [", family, model);
        switch (family) {
			case 4:
                qemu_log("486 Model %d", model);
				break;
			case 5:
				switch (model) {
					case 0:
					case 1:
					case 2:
					case 3:
					case 6:
					case 7:
                        qemu_log("K6 Model %d", model);
						break;
					case 8:
                        qemu_log("K6-2 Model 8");
						break;
					case 9:
                        qemu_log("K6-III Model 9");
						break;
					default:
                        qemu_log("K5/K6 Model %d", model);
						break;
				}
				break;
			case 6:
				switch (model) {
					case 1:
					case 2:
					case 4:
                        qemu_log("Athlon Model %d", model);
						break;
					case 3:
                        qemu_log("Duron Model 3");
						break;
					case 6:
                        qemu_log("Athlon MP/Mobile Athlon Model 6");
						break;
					case 7:
                        qemu_log("Mobile Duron Model 7");
						break;
					default:
                        qemu_log("Duron/Athlon Model %d", model);
						break;
				}
				break;

			default:
                qemu_log("Unknown");
				break;
			}

        qemu_log("]\n");
    }
    cpuid(0x80000000, extended, unused, unused, unused);
    if (extended == 0) {
        return 0;
    }
    if (extended >= 0x80000002) {
        unsigned int j;
        if (silent == 0){
            qemu_log("Detected Processor Name: ");
        }
        for (j = 0x80000002; j <= 0x80000004; j++) {
            cpuid(j, eax, ebx, ecx, edx);
            if (silent == 0){

//                qemu_log(cpuinfo_printregs(eax, ebx, ecx, edx));
                memset(brandAllName,0,128);
            }
        }
        if (silent == 0){
            qemu_log("\n");
        }
    }
    if (extended >= 0x80000007) {
        cpuid(0x80000007, unused, unused, unused, edx);
        if ((edx & 1) && silent == 0) {
            qemu_log("Temperature Sensing Diode Detected!\n");
        }
    }
    if (silent == 0){
        //qemu_log("Stepping: %d Reserved: %d\n", stepping, reserved);
    }
    return 0;
}

size_t get_max_cpuid_count() {
	uint32_t info[4] = {0};

	cpuid(0x0,
		  info[0],
		  info[1],
		  info[2],
		  info[3]
	);

	return info[0];
}