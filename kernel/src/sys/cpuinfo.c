/**
 * @file sys/cpuinfo.c
 * @author Пиминов Никита (nikita.piminoff@yandex.ru)
 * @brief Определение процессора
 * @version 0.3.2
 * @date 2022-10-01
 * @copyright Copyright SayoriOS Team (c) 2022-2023
 */
#include <kernel.h>
#include <sys/cpuinfo.h>

char brandAllName[128] = {0};                ///< Название процессора

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
 * @param bool silent - Тихий режим
 *
 * @return int - Тип процессора (0 - Unknown | 1 - Intel | 2 - AMD | 3 - VMWare)
 *
 * @warning Не для личного использования. Если вы хотите получить название процессора используйте функцию getNameBrand()
 */
int detect_cpu(bool silent) {
    int type = 0;
    size_t ebx, unused;
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
        //tty_printf("Unknown x86 CPU Detected\n");
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
 * @brief [CPUInfo] Печать регистров
 *
 * @param int eax - Регистр 1
 * @param int ebx - Регистр 2
 * @param int ecx - Регистр 3
 * @param int edx - Регистр 4
 *
 * @return char* - Имя процессора
 *
 * @warning Не для личного использования. Если вы хотите получить название процессора используйте функцию getNameBrand()
 */
char* cpuinfo_printregs(int eax, int ebx, int ecx, int edx) {
    int j;
    char *string = kmalloc(18);
    memset(string, 0, 18);
    string[17] = 0;
    for (j = 0; j < 4; j++) {
        string[j] = eax >> (8 * j);
        string[j + 4] = ebx >> (8 * j);
        string[j + 8] = ecx >> (8 * j);
        string[j + 12] = edx >> (8 * j);
    }
    //tty_printf("%s",string);
    //memset(brandAllName,0,128);
    strcat(brandAllName,string);
    //qemu_log("%d | %s",strlen(brandAllName),brandAllName);
    return string;
}

/**
 * @brief Получение информации о процессоре Intel
 *
 * @param bool silent - Тихий режим
 *
 * @return int - 0
 *
 * @warning Не для личного использования. Если вы хотите получить название процессора используйте функцию getNameBrand()
 */
int do_intel(bool silent) {
    if (silent == 0){
        tty_printf("Detected Intel CPU.\nIntel-specific features:\n");
    }
    unsigned long eax, ebx, ecx, edx, max_eax, signature, unused;
    int model, family, type, brand, stepping, reserved;
    int extended_family = -1;
    cpuid(1, eax, ebx, unused, unused);
    model = (eax >> 4) & 0xf;
    family = (eax >> 8) & 0xf;
    type = (eax >> 12) & 0x3;
    brand = ebx & 0xff;
    stepping = eax & 0xf;
    reserved = eax >> 14;
    signature = eax;
    if (silent == 0){
        tty_printf("Type %d - ", type);
        switch (type) {
        case 0:
            tty_printf("Original OEM");
            break;
        case 1:
            tty_printf("Overdrive");
            break;
        case 2:
            tty_printf("Dual-capable");
            break;
        case 3:
            tty_printf("Reserved");
            break;
        }

        tty_printf("\n");

        tty_printf("Family %d - ", family);
        switch (family) {
        case 3:
            tty_printf("i386");
            break;
        case 4:
            tty_printf("i486");
            break;
        case 5:
            tty_printf("Pentium");
            break;
        case 6:
            tty_printf("Pentium Pro");
            break;
        case 15:
            tty_printf("Pentium 4");
        }

        tty_printf("\n");

        if (family == 15) {
            extended_family = (eax >> 20) & 0xff;
            tty_printf("Extended family %d\n", extended_family);
        }
        tty_printf("Model %d - ", model);

        switch (family) {
        case 3:
            break;
        case 4:
            switch (model) {
            case 0:
            case 1:
                tty_printf("DX");
                break;
            case 2:
                tty_printf("SX");
                break;
            case 3:
                tty_printf("487/DX2");
                break;
            case 4:
                tty_printf("SL");
                break;
            case 5:
                tty_printf("SX2");
                break;
            case 7:
                tty_printf("Write-back enhanced DX2");
                break;
            case 8:
                tty_printf("DX4");
                break;
            }
            break;
        case 5:
            switch (model) {
            case 1:
                tty_printf("60/66");
                break;
            case 2:
                tty_printf("75-200");
                break;
            case 3:
                tty_printf("for 486 system");
                break;
            case 4:
                tty_printf("MMX");
                break;
            }
            break;
        case 6:
            switch (model) {
            case 1:
                tty_printf("Pentium Pro");
                break;
            case 3:
                tty_printf("Pentium II Model 3");
                break;
            case 5:
                tty_printf("Pentium II Model 5/Xeon/Celeron");
                break;
            case 6:
                tty_printf("Celeron");
                break;
            case 7:
                tty_printf("Pentium III/Pentium III Xeon - external L2 cache");
                break;
            case 8:
                tty_printf("Pentium III/Pentium III Xeon - internal L2 cache");
                break;
            }
            break;
        case 15:
            break;
        }

        tty_printf("\n");
    }
    cpuid(0x80000000, max_eax, unused, unused, unused);

    /* Quok said: If the max extended eax value is high enough to support the processor brand string
    (values 0x80000002 to 0x80000004), then we'll use that information to return the brand information.
    Otherwise, we'll refer back to the brand tables above for backwards compatibility with older processors.
    According to the Sept. 2006 Intel Arch Software Developer's Guide, if extended eax values are supported,
    then all 3 values for the processor brand string are supported, but we'll test just to make sure and be safe. */

    if (max_eax >= 0x80000004) {
        if (silent == 0){
            tty_printf("Brand: ");
            tty_printf("%s\n",brandAllName);
            memset(brandAllName,0,128);
        }
        if (max_eax >= 0x80000002) {
            cpuid(0x80000002, eax, ebx, ecx, edx);
            (cpuinfo_printregs(eax, ebx, ecx, edx));
        }
        if (max_eax >= 0x80000003) {
            cpuid(0x80000003, eax, ebx, ecx, edx);
            (cpuinfo_printregs(eax, ebx, ecx, edx));
        }
        if (max_eax >= 0x80000004) {
            cpuid(0x80000004, eax, ebx, ecx, edx);
            (cpuinfo_printregs(eax, ebx, ecx, edx));
        }
        if (silent == 0){
            tty_printf("\n");
        }
    } else if (brand > 0 && silent == 0) {
        tty_printf("Brand %d - ", brand);
        if (brand < 0x18) {
            if (signature == 0x000006B1 || signature == 0x00000F13) {
                tty_printf("%s\n", Intel_Other[brand]);
            } else {
                tty_printf("%s\n", Intel[brand]);
            }
        } else {
            tty_printf("Reserved\n");
        }
    }
    if (silent == 0){
        tty_printf("Stepping: %d Reserved: %d\n", stepping, reserved);
    }
    return 0;
}

/**
 * @brief Получение информации о процессоре AMD
 *
 * @param bool silent - Тихий режим
 *
 * @return int - 0
 *
 * @warning Не для личного использования. Если вы хотите получить название процессора используйте функцию getNameBrand()
 */
int do_amd(bool silent) {
    if (silent == 0){
        tty_printf("Detected AMD CPU. \nAMD-specific features:\n");
    }
    unsigned long extended, eax, ebx, ecx, edx, unused;
    int family, model, stepping, reserved;
    cpuid(1, eax, unused, unused, unused);

    model = (eax >> 4) & 0xf;
    family = (eax >> 8) & 0xf;
    stepping = eax & 0xf;
    reserved = eax >> 12;
    if (silent == 0){
        tty_printf("Family: %d Model: %d [", family, model);
        switch (family) {
        case 4:
            tty_printf("486 Model %d", model);
            break;
        case 5:
            switch (model) {
            case 0:
            case 1:
            case 2:
            case 3:
            case 6:
            case 7:
                tty_printf("K6 Model %d", model);
                break;
            case 8:
                tty_printf("K6-2 Model 8");
                break;
            case 9:
                tty_printf("K6-III Model 9");
                break;
            default:
                tty_printf("K5/K6 Model %d", model);
                break;
            }
            break;
        case 6:
            switch (model) {
            case 1:
            case 2:
            case 4:
                tty_printf("Athlon Model %d", model);
                break;
            case 3:
                tty_printf("Duron Model 3");
                break;
            case 6:
                tty_printf("Athlon MP/Mobile Athlon Model 6");
                break;
            case 7:
                tty_printf("Mobile Duron Model 7");
                break;
            default:
                tty_printf("Duron/Athlon Model %d", model);
                break;
            }
            break;
        }

        tty_printf("]\n");
    }
    cpuid(0x80000000, extended, unused, unused, unused);
    if (extended == 0) {
        return 0;
    }
    if (extended >= 0x80000002) {
        unsigned int j;
        if (silent == 0){
            tty_printf("Detected Processor Name: ");
        }
        for (j = 0x80000002; j <= 0x80000004; j++) {
            cpuid(j, eax, ebx, ecx, edx);
            if (silent == 0){

                tty_printf(cpuinfo_printregs(eax, ebx, ecx, edx));
                memset(brandAllName,0,128);
            }
        }
        if (silent == 0){
            tty_printf("\n");
        }
    }
    if (extended >= 0x80000007) {
        cpuid(0x80000007, unused, unused, unused, edx);
        if (edx & 1 && silent == 0) {
            tty_printf("Temperature Sensing Diode Detected!\n");
        }
    }
    if (silent == 0){
        tty_printf("Stepping: %d Reserved: %d\n", stepping, reserved);
    }
    return 0;
}
