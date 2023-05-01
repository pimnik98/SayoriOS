/**
 * @defgroup pci Драйвер PCI (Peripheral Component Interconnect)
 * @file drv/pci.c
 * @author Пиминов Никита (nikita.piminoff@yandex.ru), Арен Елчинян (SynapseOS)
 * @brief Драйвер PCI (Peripheral Component Interconnect)
 * @version 0.3.2
 * @date 2023-01-14
 * @copyright Copyright SayoriOS Team (c) 2022-2023
 */
#include <kernel.h>
#include <lib/stdio.h>
#include <io/ports.h>
#include <drv/pci.h>


/**
 * @brief [PCI] Чтение 16-битных полей из пространства механизма конфигураций 1
 *
 * @param uint8_t bus  Шина
 * @param uint8_t slot  Слот
 * @param uint8_t function  Функция
 * @param uint8_t offset Отступ
 *
 * @warning Когда доступ к конфигурации пытается выбрать несуществующее устройство, хост-мост завершает доступ без ошибок, удаляя все данные при записи и возвращая все данные при чтении.
 *
 * @return uint16_t
 */
uint16_t pci_read_confspc_word(uint8_t bus, uint8_t slot, uint8_t function, uint8_t offset) {
    uint32_t addr;
    uint32_t bus32 = bus;
    uint32_t slot32 = slot;
    uint32_t func32 = function;
    
    addr = (uint32_t)((bus32 << 16) | (slot32 << 11) |
           (func32 << 8) | (offset & 0xfc) | ((uint32_t)0x80000000)); //yes, this line is copied from osdev

    outl(PCI_ADDRESS_PORT, addr);

    return (uint16_t)((inl(PCI_DATA_PORT) >> ((offset & 2) * 8)) & 0xffff); //this too... I'm too lazy to write them
}

uint32_t pci_read32(uint8_t bus, uint8_t slot, uint8_t function, uint8_t offset) {
    uint32_t addr;
    uint32_t bus32 = bus;
    uint32_t slot32 = slot;
    uint32_t func32 = function;
    addr = (uint32_t)((bus32 << 16) | (slot32 << 11) |
           (func32 << 8) | (offset & 0xfc) | ((uint32_t)0x80000000)); //yes, this line is copied from osdev
    outl(PCI_ADDRESS_PORT, addr);
    return ((inl(PCI_DATA_PORT) >> ((offset & 2) * 8)) & 0xffff); //this too... I'm too lazy to write them
}


/**
 * @brief [PCI] Категория устройств
 *
 */
static struct {
    uint8_t class, subclass;
    const char *name;
} pci_device_type_strings[] = {
    {0x00, 0x00, "Неизвестное устройство"},
    {0x00, 0x01, "VGA-совместимое устройство"},
    {0x01, 0x00, "Контроллер шины SCSI"},
    {0x01, 0x01, "IDE-контроллер"},
    {0x01, 0x02, "Контроллер гибких дисков"},
    {0x01, 0x03, "Контроллер шины IPI"},
    {0x01, 0x04, "RAID-контроллер"},
    {0x01, 0x05, "Контроллер АТА"},
    {0x01, 0x06, "SATA-контроллер"},
    {0x01, 0x07, "Последовательный подключенный контроллер SCSI"},
    {0x01, 0x80, "Другой контроллер запоминающих устройств"},
    {0x02, 0x00, "Ethernet-контроллер"},
    {0x02, 0x01, "Контроллер Token Ring"},
    {0x02, 0x02, "FDDI-контроллер"},
    {0x02, 0x03, "Контроллер банкомата"},
    {0x02, 0x04, "ISDN-контроллер"},
    {0x02, 0x05, "Контроллер WorldFip"},
    {0x02, 0x06, "PICMG 2.14 Мультивычисления"},
    {0x02, 0x80, "Другой сетевой контроллер"},
    {0x03, 0x00, "VGA-совместимый контроллер"},
    {0x03, 0x01, "XGA-контроллер"},
    {0x03, 0x02, "3D контроллер"},
    {0x03, 0x80, "Другой контроллер дисплея"},
    {0x04, 0x00, "Видео-устройство"},
    {0x04, 0x01, "Аудио-устройство"},
    {0x04, 0x02, "Компьютерное телефонное устройство"},
    {0x04, 0x80, "Другое мультимедийное устройство"},
    {0x05, 0x00, "Контроллер оперативной памяти"},
    {0x05, 0x01, "Флэш-контроллер"},
    {0x05, 0x80, "Другой контроллер памяти"},
    {0x06, 0x00, "Хост-мост"},
    {0x06, 0x01, "ISA мост"},
    {0x06, 0x02, "EISA мост"},
    {0x06, 0x03, "MCA мост"},
    {0x06, 0x04, "PCI-to-PCI мост"},
    {0x06, 0x05, "PCMCIA мост"},
    {0x06, 0x06, "NuBus мост"},
    {0x06, 0x07, "CardBus мост"},
    {0x06, 0x08, "RACEWay мост"},
    {0x06, 0x09, "PCI-to-PCI мост (Полупрозрачный)"},
    {0x06, 0x0A, "Хост-мост InfiniBand-PCI"},
    {0x06, 0x80, "Другое устройство моста"},
    {0x07, 0x00, "Последовательный контроллер"},
    {0x07, 0x01, "Параллельный порт"},
    {0x07, 0x02, "Многопортовый последовательный контроллер"},
    {0x07, 0x03, "Универсальный модем"},
    {0x07, 0x04, "IEEE 488.1/2 (GPIB) контроллер"},
    {0x07, 0x05, "Интеллектуальная карточка"},
    {0x07, 0x80, "Другое устройство связи"},
    {0x08, 0x00, "Программируемый контроллер прерываний"},
    {0x08, 0x01, "Контроллер прямого доступа к памяти"},
    {0x08, 0x02, "Системный таймер"},
    {0x08, 0x03, "Часы реального времени"},
    {0x08, 0x04, "Универсальный контроллер PCI с возможностью горячей замены"},
    {0x08, 0x80, "Другая системная периферия"},
    {0x09, 0x00, "Контроллер клавиатуры"},
    {0x09, 0x01, "Цифровой преобразователь"},
    {0x09, 0x02, "Контроллер мыши"},
    {0x09, 0x03, "Контроллер сканера"},
    {0x09, 0x04, "Контроллер игрового порта"},
    {0x09, 0x80, "Другой контроллер ввода"},
    {0x0A, 0x00, "Универсальная док-станция"},
    {0x0A, 0x80, "Другая док-станция"},
    {0x0B, 0x00, "Процессор i386"},
    {0x0B, 0x01, "Процессор i486"},
    {0x0B, 0x02, "Процессор Pentium"},
    {0x0B, 0x10, "Процессор Alpha"},
    {0x0B, 0x20, "Процессор PowerPC"},
    {0x0B, 0x30, "Процессор MIPS"},
    {0x0B, 0x40, "Со-процессор"},
    {0x0C, 0x00, "Контроллер FireWire"},
    {0x0C, 0x01, "Контроллер ACCESS.bus"},
    {0x0C, 0x02, "SSA Контроллер"},
    {0x0C, 0x03, "USB Контроллер"},
    {0x0C, 0x04, "Волоконный канал"},
    {0x0C, 0x05, "SMBus"},
    {0x0C, 0x06, "InfiniBand"},
    {0x0C, 0x07, "Интерфейс IPMI SMIC"},
    {0x0C, 0x08, "Интерфейс SERCOS"},
    {0x0C, 0x09, "Интерфейс CANbus"},
    {0x0D, 0x00, "iRDA-совместимый контроллер"},
    {0x0D, 0x01, "Потребительский ИК-контроллер"},
    {0x0D, 0x10, "RF Контроллер"},
    {0x0D, 0x11, "Bluetooth Контроллер"},
    {0x0D, 0x12, "Broadband Контроллер"},
    {0x0D, 0x20, "802.11a (Wi-Fi) Ethernet-контроллер"},
    {0x0D, 0x21, "802.11b (Wi-Fi) Ethernet-контроллер"},
    {0x0D, 0x80, "Другой беспроводной контроллер"},
    {0x00, 0x00, NULL} // Конец
};

/**
 * @brief [PCI] Поставщики устройств
 *
 */
static struct {
    uint16_t vendor;
    const char *name;
} pci_vendor_name_strings[] = {
    {0x8086, "Intel Corporation"},
    {0x10DE, "NVIDIA"},
    {0x0014, "Loongson Technology Corporation Limited"},
    {0x001C, "PEAK-System Technik GmbH"},
    {0x00B0, "Blue Origin, LLC"},
    {0x00BB, "Bloombase"},
    {0x0123, "General Dynamics Mission Systems, Inc."},
    {0x6688, "GUANGZHOU MAXSUN INFORMATION TECHNOLOGY CO., LTD."},
    {0x751A, "Tesla Inc."},
    {0x8080, "StoreSwift Technology Co., Ltd."},
    {0x1013, "Cirrus Logic, Inc."},
    {0x1014, "IBM"},
    {0x101E, "American Megatrends Incorporated"},
    {0x1028, "Dell Computer Corporation"},
    {0x102B, "Matrox Graphics Inc."},
    {0xA23B, "Silicon Integrated Systems"},
    {0x103C, "Hewlett Packard"},
    {0x1043, "Asustek Computer Inc."},
    {0x104C, "Texas Instruments"},
    {0x104D, "Sony Group Corporation"},
    {0x1054, "Hitachi, Ltd."},
    {0x1002, "[AMD] Advanced Micro Devices Inc."},
    {0x1022, "[AMD] Advanced Micro Devices Inc."},
    {0x10EC, "Realtek Semiconductor Corp."},
    {0x1039, "[SiS] Silicon Integrated Systems"},
    {0x0B05, "[ASUS] ASUSTek Computer, Inc."},
    {0x80EE, "[VirtualBox] InnoTek Systemberatung GmbH"},
    {0x1234, "[QEMU] Technical Corp"},
    {0x106B, "Apple Inc."},
    {0x1AF4, "Red Hat, Inc."},
    {0, NULL}
};

/**
 * @brief [PCI] Получение основной категории устройства
 *
 * @param uint8_t bus  Шина
 * @param uint8_t slot  Слот
 * @param uint8_t function  Функция
 *
 * @return uint8_t Категория устройства
 */
uint8_t pci_get_class(uint8_t bus, uint8_t slot, uint8_t function) {
    /* Сдвигаем, чтобы оставить только нужные данные в переменной */
    return (uint8_t) (pci_read_confspc_word(bus, slot, function, 10) >> 8);
}

/**
 * @brief [PCI] Получение под-категории устройства
 *
 * @param uint8_t bus  Шина
 * @param uint8_t slot  Слот
 * @param uint8_t function  Функция
 * @return uint8_t Подкатегория устройства
 */
uint8_t pci_get_subclass(uint8_t bus, uint8_t slot, uint8_t function) {
    /* Сдвигаем, чтобы оставить только нужные данные в переменной */
    return (uint8_t) pci_read_confspc_word(bus, slot, function, 10);
}

/**
 * @brief [PCI] Получение HDR-тип устройства
 *
 * @param uint8_t bus  Шина
 * @param uint8_t slot  Слот
 * @param uint8_t function  Функция
 * @return uint8_t HDR-тип
 */
uint8_t pci_get_hdr_type(uint8_t bus, uint8_t slot, uint8_t function) {
    /* Сдвигаем, чтобы оставить только нужные данные в переменной */
    return (uint8_t) pci_read_confspc_word(bus, slot, function, 7);
}

/**
 * @brief [PCI] Получение ID-поставщика
 *
 * @param uint8_t bus  Шина
 * @param uint8_t slot  Слот
 * @param uint8_t function  Функция
 * @return uint16_t ID-поставщика
 */
uint16_t pci_get_vendor(uint8_t bus, uint8_t slot, uint8_t function) {
    /* Сдвигаем, чтобы оставить только нужные данные в переменной */
    return pci_read_confspc_word(bus, slot, function, 0);
}

/**
 * @brief [PCI] Получение ID-Устройства
 *
 * @param uint8_t bus  Шина
 * @param uint8_t slot  Слот
 * @param uint8_t function  Функция
 * @return uint8_t ID-Устройства
 */
uint16_t pci_get_device(uint8_t bus, uint8_t slot, uint8_t function) {
    /* Сдвигаем, чтобы оставить только нужные данные в переменной */
    return pci_read_confspc_word(bus, slot, function, 2);
}

/**
 * @brief [PCI] Получение классификации устройства
 *
 * @param uint8_t class Группа А
 * @param uint8_t subclass Группа Б
 * @return const char * Возращает классификацию устройства
 */
const char *pci_get_device_type(uint8_t class, uint8_t subclass) {
    for (int i=0; pci_device_type_strings[i].name != NULL; i++)
        if (pci_device_type_strings[i].class == class && pci_device_type_strings[i].subclass == subclass)
            return pci_device_type_strings[i].name;
    return NULL;
}

/**
 * @brief [PCI] Получение названия поставщика
 *
 * @param uint16_t vendor Поставщик
 * @return const char * Возращает имя поставщика
 */
const char *pci_get_vendor_name(uint16_t vendor) {
    for (int i=0; pci_vendor_name_strings[i].name != NULL; i++)
        if (pci_vendor_name_strings[i].vendor == vendor)
            return pci_vendor_name_strings[i].name;
    return "unknown";
}

/**
 * @brief [PCI] ???
 *
 * @param uint8_t hdrtype ???
 * @param uint8_t bus  Шина
 * @param uint8_t slot  Слот
 * @param uint8_t function  Функция
 * @param uint8_t bar_number Номер BAR от 0 до 5
 * @param uint8_t *bar_type ???
 *
 * @todo Необходимо добавить 64-бит реализацию
 *
 * @return uint32_t ???
 */
uint32_t pci_get_bar(uint8_t hdrtype, uint8_t bus, uint8_t slot, uint8_t func, uint8_t bar_number, uint8_t *bar_type) {
    if ((hdrtype & ~0x80) == 0) {
        uint8_t off = bar_number * 2;
        uint16_t bar_low  = pci_read_confspc_word(bus, slot, func, 0x10 + off);
        uint16_t bar_high = pci_read_confspc_word(bus, slot, func, 0x10 + off + 1);
        if ((bar_low & 1) == 0) {
            if ((bar_low & ~0b110) == 0x00) // 32 бит. 64 бит нужно добавить!
            {
                uint32_t ret = (uint32_t) bar_low | (uint32_t) (bar_high << 16);
                ret &= ~0b1111;
                *bar_type = 0;
                return ret;
            }
        }
        if ((bar_low & 1) == 1) {
            uint32_t ret = (uint32_t)bar_low | (uint32_t)(bar_high << 16);
            ret &= ~0b11;
            *bar_type = 1;
            return ret;
        }
    }
    return 0;
}

void pci_write(uint8_t bus, uint8_t slot, uint8_t func, uint32_t offset, uint32_t value) {
    uint32_t addr = (bus << 16) | (slot << 11) | (func << 8) | (offset & 0xfc) | ((uint32_t)0x80000000);
	// uint32_t addr = 0x80000000 | (bus << 16) | (slot << 11) | (func << 8) | offset;
    // qemu_log("PCI Write to: %x; Value: %d (%x)", addr, value, value);
    // Tell where we want to write
	outl(PCI_ADDRESS_PORT, addr);
	// Value to write
	outl(PCI_DATA_PORT, value);
    // qemu_log("Ok.");
}

/**
 * @brief [PCI] Поиск устройства по ID-поставшика и устройства
 *
 * @param uint16_t vendor ID-Поставщика
 * @param uint16_t device ID-Устройства
 * @param uint8_t *bus_ret
 * @param uint8_t *slot_ret
 * @param uint8_t *func_ret
 */
void pci_find_device(uint16_t vendor, uint16_t device, uint8_t *bus_ret, uint8_t *slot_ret, uint8_t *func_ret) {
    for (uint32_t bus=0; bus<256; bus++)
        for (uint32_t slot=0; slot<32; slot++)
            for (uint32_t func=0; func<8; func++)
                if (pci_get_device(bus, slot, func) == device && pci_get_vendor(bus, slot, func) == vendor) {
                    *bus_ret  = bus;
                    *slot_ret = slot;
                    *func_ret = func;
                    return;
                }
    *bus_ret = *slot_ret = *func_ret = 0xFF;
}

/**
 * @brief [PCI] Ищет все устройства и выводит на экран
 */
void pci_print_list() {
    uint8_t clid;
    uint8_t sclid;
    uint8_t hdrtype;
    uint16_t vendor;
    uint16_t device;

    tty_printf("PCI список устройств:\n");
    for (uint32_t bus = 0; bus < 256; bus++) {
        for (uint32_t slot = 0; slot < 32; slot++) {
            uint32_t func = 0;

            clid = pci_get_class(bus, slot, func);
            sclid = pci_get_subclass(bus, slot, func);
            vendor = pci_get_vendor(bus, slot, func);
            hdrtype = pci_get_hdr_type(bus, slot, func);
            device = pci_get_device(bus, slot, func);

            if (clid != 0xFF && vendor != 0xFFFF) {
                qemu_log("%u:%u.%u %s: %s (%x), девайс: %x", bus, slot, func, pci_get_device_type(clid, sclid), pci_get_vendor_name(vendor),vendor, device);
                tty_printf("\t%u:%u.%u %s: %s (%x), девайс: %x\n", bus, slot, func, pci_get_device_type(clid, sclid), pci_get_vendor_name(vendor),vendor, device);
            }

            if ((hdrtype & 0x80) == 0) {
                for (func = 1; func < 8; func++) {
                    clid = pci_get_class(bus, slot, func);
                    sclid = pci_get_subclass(bus, slot, func);
                    vendor = pci_get_vendor(bus, slot, func);
                    device = pci_get_device(bus, slot, func);

                    if (clid != 0xFF && vendor != 0xFFFF) {
                        qemu_log("%u:%u.%u %s: %s (%x), девайс: %x", bus, slot, func, pci_get_device_type(clid, sclid), pci_get_vendor_name(vendor),vendor, device);
                        tty_printf("\t%u:%u.%u %s: %s (%x), девайс: %x\n", bus, slot, func, pci_get_device_type(clid, sclid), pci_get_vendor_name(vendor),vendor, device);
                    }
                }
            }
        }
    }
}
