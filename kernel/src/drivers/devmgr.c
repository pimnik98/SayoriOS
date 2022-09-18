#include <kernel.h>
#include <drivers/devmgr.h>
devmgr_t dev[1024] = {0};
uint32_t count = 0;

static devmgr_cat_t dCat[128] = {
    {0x0,0x0,0xFFFF,"Unclassified","Non-VGA-Compatible Unclassified Device",""},
    {0x0,0x1,0xFFFF,"Unclassified","VGA-Compatible Unclassified Device",""},
    {0x1,0x0,0xFFFF,"Mass Storage Controller","SCSI Bus Controller",""},
    {0x1,0x1,0x0,"Mass Storage Controller","IDE Controller","ISA Compatibility mode-only controller"},
    {0x1,0x1,0x5,"Mass Storage Controller","IDE Controller","PCI native mode-only controller"},
    {0x1,0x1,0xA,"Mass Storage Controller","IDE Controller","ISA Compatibility mode controller, supports both channels switched to PCI native mode "},
    {0x1,0x1,0xF,"Mass Storage Controller","IDE Controller","PCI native mode controller, supports both channels switched to ISA compatibility mode "},
    {0x1,0x1,0x80,"Mass Storage Controller","IDE Controller","ISA Compatibility mode-only controller, supports bus mastering"},
    {0x1,0x1,0x85,"Mass Storage Controller","IDE Controller","PCI native mode-only controller, supports bus mastering "},
    {0x1,0x1,0x8A,"Mass Storage Controller","IDE Controller","ISA Compatibility mode controller, supports both channels switched to PCI native mode, supports bus mastering "},
    {0x1,0x1,0x8F,"Mass Storage Controller","IDE Controller","PCI native mode controller, supports both channels switched to ISA compatibility mode, supports bus mastering "},
    {0x1,0x2,0xFFFF,"Mass Storage Controller","Floppy Disk Controller",""},
    {0x1,0x3,0xFFFF,"Mass Storage Controller","IPI Bus Controller",""},
    {0x1,0x4,0xFFFF,"Mass Storage Controller","RAID Controller",""},
    {0x1,0x5,0x20,"Mass Storage Controller","ATA Controller","Single DMA"},
    {0x1,0x5,0x30,"Mass Storage Controller","ATA Controller","Chained DMA"},
    {0x1,0x6,0x0,"Mass Storage Controller","Serial ATA Controller","Vendor Specific Interface"},
    {0x1,0x6,0x1,"Mass Storage Controller","Serial ATA Controller","AHCI 1.0"},
    {0x1,0x6,0x2,"Mass Storage Controller","Serial ATA Controller","Serial Storage Bus"},
    {0x1,0x7,0x0,"Mass Storage Controller","Serial Attached SCSI Controller","SAS"},
    {0x1,0x7,0x1,"Mass Storage Controller","Serial Attached SCSI Controller","Serial Storage Bus"},
    {0x1,0x8,0x1,"Mass Storage Controller","Non-Volatile Memory Controller","NVMHCI"},
    {0x1,0x8,0x2,"Mass Storage Controller","Non-Volatile Memory Controller","NVM Express"},
    {0x1,0x80,0xFFFF,"Mass Storage Controller","Other",""},
    {0x2,0x0,0xFFFF,"Network Controller","Ethernet Controller",""},
    {0x2,0x1,0xFFFF,"Network Controller","Token Ring Controller",""},
    {0x2,0x2,0xFFFF,"Network Controller", "FDDI Controller",""},
    {0x2,0x3,0xFFFF,"Network Controlle", "ATM Controller",""},
    {0x2,0x4,0xFFFF,"Network Controlle", "ISDN Controller",""},
    {0x2,0x5,0xFFFF,"Network Controller", "WorldFip Controller",""},
    {0x2,0x6,0xFFFF,"Network Controller", "PICMG 2.14 Multi Computing Controller",""},
    {0x2,0x7,0xFFFF,"Network Controller", "Infiniband Controller",""},
    {0x2,0x8,0xFFFF,"Network Controller", "Fabric Controller",""},
    {0x2,0x80,0xFFFF,"Network Controller", "Other",""},
    {0x3,0x0,0x0,"Display Controller", "VGA Controller",""},
    {0x3,0x0,0x1,"Display Controller","8514-Compatible Controller",""},
    {0x3,0x1,0xFFFF,"Display Controller","XGA Controller",""},
    {0x3,0x2,0xFFFF,"Display Controller","3D Controller (Not VGA-Compatible)",""},
    {0x3,0x80,0xFFFF,"Display Controller", "Other",""},
    {0x4,0x0,0xFFFF,"Multimedia Controller","Multimedia Video Controller",""},
    {0x4,0x1,0xFFFF,"Multimedia Controller","Multimedia Audio Controller",""},
    {0x4,0x2,0xFFFF,"Multimedia Controller","Computer Telephony Device",""},
    {0x4,0x3,0xFFFF,"Multimedia Controller","Audio Device",""},
    {0x4,0x80,0xFFFF,"Multimedia Controller","Other",""},
    {0x5,0x0,0xFFFF,"Memory Controller","RAM Controller",""},
    {0x5,0x1,0xFFFF,"Flash Controller","Flash Controller",""},
    {0x5,0x80,0xFFFF,"Memory Controller","Other",""},
	{0x6,0x0,0xFFFF,"Bridge","Host Bridge",""},
	{0x6,0x1,0xFFFF,"Bridge","ISA Bridge",""},
	{0x6,0x2,0xFFFF,"Bridge","EISA Bridge",""},
	{0x6,0x3,0xFFFF,"Bridge","MCA Bridge",""},
	{0x6,0x4,0x0,"Bridge","PCI-to-PCI Bridge","Normal Decode"},
	{0x6,0x4,0x1,"Bridge","PCI-to-PCI Bridge","Subtractive Decode"},
	{0x6,0x5,0xFFFF,"Bridge","PCMCIA Bridge ",""},
	{0x6,0x6,0xFFFF,"Bridge","NuBus Bridge",""},
	{0x6,0x7,0xFFFF,"Bridge","CardBus Bridge",""},
	{0x6,0x8,0x0,"Bridge","RACEway Bridge","Transparent Mode"},
	{0x6,0x8,0x1,"Bridge","RACEway Bridge","Endpoint Mode"},
	{0x6,0x9,0x40,"Bridge","PCI-to-PCI Bridge","Semi-Transparent, Primary bus towards host CPU"},
	{0x6,0x89,0x80,"Bridge","PCI-to-PCI Bridge","emi-Transparent, Secondary bus towards host CPU "},
	{0x6,0x0A,0xFFFF,"Bridge","InfiniBand-to-PCI Host Bridge ",""},
	{0x6,0x80,0xFFFF,"Bridge","Other",""},
    {0xC,0x0,0x0,"Serial Bus Controller","FireWire (IEEE 1394) Controller","Generic"},
    {0xC,0x0,0x10,"Serial Bus Controller","FireWire (IEEE 1394) Controller","OHCI"},
    {0xC,0x1,0xFFFF,"Serial Bus Controller","ACCESS Bus Controller",""},
    {0xC,0x2,0xFFFF,"Serial Bus Controller","SSA",""},
    {0xC,0x3,0x0,"Serial Bus Controller","USB Controller","UHCI Controller"},
    {0xC,0x3,0x10,"Serial Bus Controller","USB Controller","OHCI Controller"},
    {0xC,0x3,0x20,"Serial Bus Controller","USB Controller","EHCI (USB2) Controller"},
    {0xC,0x3,0x30,"Serial Bus Controller","USB Controller","XHCI (USB3) Controller"},
    {0xC,0x3,0x80,"Serial Bus Controller","USB Controller","Unspecified"},
    {0xC,0x3,0xFE,"Serial Bus Controller","USB Controller","USB Device"},
    {0xC,0x4,0xFFFF,"Serial Bus Controller","Fibre Channel",""},
    {0xC,0x5,0xFFFF,"Serial Bus Controller","SMBus Controller",""},
    {0xC,0x6,0xFFFF,"Serial Bus Controller","InfiniBand Controller",""},
    {0xC,0x7,0x0,"Serial Bus Controller","IPMI Interface","SMIC"},
    {0xC,0x7,0x1,"Serial Bus Controller","IPMI Interface","Keyboard Controller Style"},
    {0xC,0x7,0x2,"Serial Bus Controller","IPMI Interface","Block Transfer"},
    {0xC,0x8,0xFFFF,"Serial Bus Controller","SERCOS Interface (IEC 61491) ",""},
    {0xC,0x9,0xFFFF,"Serial Bus Controller","CANbus Controller",""},
    {0xC,0x80,0xFFFF,"Serial Bus Controller","Other",""}
};

/**
 * @brief Возращает количество подключенных устройств
 * @return uint32_t - Количество подключенных устройств
 */
uint32_t getCountDevices(){
    return count;
}

/**
 * @brief Получить название устройства по ID Поставщика и ID устройства
 * @param uint32_t VendorID - ID Поставщика
 * @param uint32_t DeviceID - ID Устройства
 * @return char* Название устройства
 */
char* getDeviceNameForSearch(uint32_t VendorID,uint32_t DeviceID){
    char * pci_list = "/initrd/etc/pci/devices";
    FILE* pdevice = fopen(pci_list,"r");
    if (ferror(pdevice) == 0){
        char * bf = fread(pdevice);
        fclose(pdevice);
        uint32_t c1 = str_cdsp(bf,"\n");
        char* out[128] = {0};
        str_split(bf,out,"\n");
        for(int i = 0; c1 > i; i++){
            char* aut[3] = {0};
            uint32_t c2 = str_cdsp(out[i],";");
            if (c2 > 3){
                continue;
            }
            str_split(out[i],aut,";");
            //qemu_log("[DevMgr] %x == %x && %x == %x",atoi(aut[0]),VendorID,atoi(aut[1]),DeviceID);
            if (atoi(aut[0]) == VendorID && atoi(aut[1]) == DeviceID){
                return aut[2];
            }
            continue;
        }
    }
    return "Unknown";
}

/**
 * @brief Получить название устройства по индексу
 * @param uint32_t id - Номер индекса, установленого устройства
 * @return char* Название устройства
 */
char* getDeviceName(uint32_t id){
    return getDeviceNameForSearch(dev[id].vendor,dev[id].device);
}

uint32_t findCatDevice(uint32_t device,uint32_t c, uint32_t s, uint32_t p){
    if (dev[device].category != -1){
        return dev[device].category;
    }
    for(uint32_t i = 0;i < 128; i++){
        devmgr_cat_t cat = dCat[i];
        if (cat.classID == c && cat.subClass == s && (cat.pifID == 0xFFFF || cat.pifID == p)){
            dev[device].category = i;
            qemu_log("[DevMgr] [ID:%d] Category: [%d-%d-%d] %s %s %s",device,c,s,p,cat.nameCat,cat.nameSubCat,cat.namePif);
            return i;
        }
    }
    dev[device].category = 0;
    return 0;
    qemu_log("[DevMgr] [ID:%d] Category: [%d-%d-%d] %s %s %s",device,c,s,p,"Unknown","","");
}

/**
 * @brief Информация о категории в котором состоит устройство
 * @param uint32_t id - Номер индекса, установленого устройства
 * @param uint32_t key - Ключ параметра: DEVMGR_KEY_CLASS || DEVMGR_KEY_SUBCLASS || DEVMGR_KEY_PROGIF
 * @return char* Название категории
 */
char* getCategoryDevice(uint32_t id,uint32_t key){
    uint32_t i = findCatDevice(id,dev[id].vendor,dev[id].device,dev[id].progIF);
    if (key == DEVMGR_KEY_CLASS){
        return dCat[i].nameCat;
    } else if (key == DEVMGR_KEY_SUBCLASS){
        return dCat[i].nameSubCat;
    } else if (key == DEVMGR_KEY_PROGIF){
        return dCat[i].namePif;
    }
    return "???";
}
/**
 * @brief Информация об устройстве
 * @param uint32_t id - Номер индекса, установленого устройства
 * @param uint32_t key - Ключ параметра: DEVMGR_KEY_VENDORID || DEVMGR_KEY_CLASS || DEVMGR_KEY_STATE || DEVMGR_KEY_CATEGORY || DEVMGR_KEY_CLASS || DEVMGR_KEY_SUBCLASS || DEVMGR_KEY_PROGIF
 * @return char* Название категории
 */
uint32_t getDeviceInfo(uint32_t id,uint32_t key){
    if (key == DEVMGR_KEY_VENDORID){
        return dev[id].vendor;
    } else if (key == DEVMGR_KEY_DEVICEID){
        return dev[id].device;
    } else if (key == DEVMGR_KEY_CLASS){
        return dev[id].classID;
    } else if (key == DEVMGR_KEY_SUBCLASS){
        return dev[id].subClass;
    } else if (key == DEVMGR_KEY_PROGIF){
        return dev[id].progIF;
    } else if (key == DEVMGR_KEY_STATE){
        return dev[id].state;
    } else if (key == DEVMGR_KEY_CATEGORY){
        return dev[id].category;
    } else {
        return 0;
    }
    return 0;
}


/**
 * @brief Проверяет установлено ли уже устройство
 * @param uint32_t VendorID - ID Поставщика
 * @param uint32_t DeviceID - ID Устройства
 * @return uint32_t -1 если устройство не найдено, или его индекс
 */
uint32_t foundDevice(uint32_t VendorID,uint32_t DeviceID){
    for(int i= 0;i < count;i++){
        if (dev[i].vendor == VendorID && dev[i].device == DeviceID){
            return i;
        }
    }
    return -1;
}

/**
 * @brief Регистрирует устройство в системе
 * @param uint32_t VendorID - ID Поставщика
 * @param uint32_t DeviceID - ID Устройства
 * @return uint32_t -1 если устройство не найдено, или индекс установленого устройства
 */
uint32_t registerDevice(uint32_t VendorID,uint32_t DeviceID){
    qemu_log("[DevMgr] [ID: %d] Register Device [%x&%x]",count,VendorID,DeviceID);
    uint32_t found = foundDevice(VendorID, DeviceID);
    if (found != -1){
        qemu_log("[DevMgr] The device has already been added.");
        return found;
    }
    pci_dev_t pdt;
	pdt = pci_get_device(VendorID, DeviceID, -1);
    if (pdt.bits == -1){
        qemu_log("[DevMgr] Sorry, but this device is not available and will be ignored.");
        return -1;
    }
    dev[count].vendor = VendorID;
    dev[count].device = DeviceID;
    dev[count].classID = pci_read(pdt, PCI_CLASS);
    dev[count].subClass = pci_read(pdt, PCI_SUBCLASS);
    dev[count].progIF = pci_read(pdt, PCI_PROG_IF);
    dev[count].state = pdt.bits;
    dev[count].category = -1;
    //qemu_log("[DevMgr] [ID:%d] Vendor: %s | Name: %s",count,getVendorName(VendorID),getDeviceName(VendorID,DeviceID));

    findCatDevice(count,dev[count].classID,dev[count].subClass,dev[count].progIF);
    count++;
    return (count-1);
    // pci_read(pdt, PCI_CLASS),pci_read(pdt, PCI_SUBCLASS)
}
