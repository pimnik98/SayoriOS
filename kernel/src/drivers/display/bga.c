#include <kernel.h>
#define VBE_DISPI_BANK_ADDRESS 0xA0000
#define VBE_DISPI_LFB_PHYSICAL_ADDRESS 0xE0000000
#define VBE_DISPI_BANK_SIZE_KB 64

#define VBE_DISPI_ID0 0xB0C0
#define VBE_DISPI_ID1 0xB0C1
#define VBE_DISPI_ID2 0xB0C2
#define VBE_DISPI_ID3 0xB0C3
#define VBE_DISPI_ID4 0xB0C4
#define VBE_DISPI_ID5 0xB0C5

#define VBE_DISPI_VBOX 0x8000000

#define VBE_DISPI_IOPORT_INDEX 0x01CE
#define VBE_DISPI_IOPORT_DATA 0x01CF

#define VBE_DISPI_INDEX_ID 0x0
#define VBE_DISPI_INDEX_XRES 0x1
#define VBE_DISPI_INDEX_YRES 0x2
#define VBE_DISPI_INDEX_BPP 0x3
#define VBE_DISPI_INDEX_ENABLE 0x4
#define VBE_DISPI_INDEX_BANK 0x5
#define VBE_DISPI_INDEX_VIRT_WIDTH 0x6
#define VBE_DISPI_INDEX_VIRT_HEIGHT 0x7
#define VBE_DISPI_INDEX_X_OFFSET 0x8
#define VBE_DISPI_INDEX_Y_OFFSET 0x9

#define VBE_DISPI_MAX_XRES 2560
#define VBE_DISPI_MAX_YRES 1600
#define VBE_DISPI_MAX_BPP 32

#define VBE_DISPI_DISABLED 0x00
#define VBE_DISPI_ENABLED 0x01
#define VBE_DISPI_GETCAPS 0x02
#define VBE_DISPI_8BIT_DAC 0x20
#define VBE_DISPI_LFB_ENABLED 0x40
#define VBE_DISPI_NOCLEARMEM 0x80

#define VBE_DISPI_BPP_4 0x04
#define VBE_DISPI_BPP_8 0x08
#define VBE_DISPI_BPP_15 0x0F
#define VBE_DISPI_BPP_16 0x10
#define VBE_DISPI_BPP_24 0x18
#define VBE_DISPI_BPP_32 0x20

#define VBE_DISPI_ID_VBOX_VIDEO 0xBE00  // графическое устройство поддерживает ускорение видео VirtualBox (VBVA).
#define VBE_DISPI_ID_HGSMI 0xBE01   // графическое устройство поддерживает интерфейс общей памяти VirtualBox Host-Guest.
#define VBE_DISPI_ID_ANYX 0xBE02    // графическое устройство поддерживает любую ширину экрана.

#define VBE_DISPI_INDEX_VBOX_VIDEO 0x0A // используется для чтения информации о конфигурации и записи команд на хост (иногда).
#define VBE_DISPI_INDEX_FB_BASE_HI 0x0B // содержит старшие 16 бит адреса линейного буфера кадра (младшие 16 бит равны нулю).

uint32_t mountPoint;    // Место адресации запросов

void bgaWriteData(uint32_t key,uint32_t value){
    qemu_log("[bWrite] %x => %x",key,value);
    outs(VBE_DISPI_IOPORT_INDEX, key);
    outs(VBE_DISPI_IOPORT_DATA, value);
}

uint32_t bgaReadData(uint32_t key){
    outs(VBE_DISPI_IOPORT_INDEX, key);
    uint32_t data = ins(VBE_DISPI_IOPORT_DATA);
    qemu_log("[bGet] %x => %x",key,data);
    return data;
}

void bgaDisabled(){
    bgaWriteData(VBE_DISPI_INDEX_ENABLE,VBE_DISPI_DISABLED);
}

void bgaEnabled(){
    bgaWriteData(VBE_DISPI_INDEX_ENABLE,VBE_DISPI_ENABLED);
}

void bgaChangeResize(uint32_t w, uint32_t h, uint32_t bpp){
    bgaWriteData(VBE_DISPI_INDEX_XRES,w);
    bgaWriteData(VBE_DISPI_INDEX_YRES,h);
    bgaWriteData(VBE_DISPI_INDEX_BPP,bpp);
}

void bgaInit(){
    uint32_t status = bgaReadData(VBE_DISPI_INDEX_ID);
    uint32_t x = bgaReadData(VBE_DISPI_INDEX_XRES);
    uint32_t y = bgaReadData(VBE_DISPI_INDEX_YRES);
    uint32_t b = bgaReadData(VBE_DISPI_INDEX_BPP);
    if (status == VBE_DISPI_ID5){
        qemu_log("[BGA] `BGA` driver version 5.0 detected.");
    } else if (status == VBE_DISPI_ID4){
        qemu_log("[BGA] `BGA` driver version 4.0 detected.");
    } else {
        qemu_log("[BGA] Video driver `BGA` was ignored by the system. Possible Cause: The video adapter is not supported by your device, or an older version of the video adapter is being used.");
        return;
    }
    mountPoint = pci_read(pci_get_device(0x1234, 0x1111, -1), PCI_BAR0);
    qemu_log("[BGA] Video adapter address space: %x",mountPoint);
    qemu_log("[BGA] MaxX:%d MaxY:%d BPP:%d",x,y,b);

}

void bgaTest(){
    uint32_t x = 800;
    uint32_t y = 600;
    uint32_t b = VBE_DISPI_BPP_32;
    bgaDisabled();
    bgaChangeResize(x, y , b);
    bgaEnabled();

    //pci_dev_t BGA_device = pci_get_device(0x1234, BGA_FOUND, -1);
    //uint32_t pci_command_reg = pci_read(BGA_device, PCI_BAR0);
   // qemu_log("[BGA] %x / %x",BGA_FOUND,pci_command_reg);

    bgaDriverInit(x,y,b,mountPoint);
}
