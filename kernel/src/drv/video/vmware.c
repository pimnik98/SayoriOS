/**
 * @file drv/video/vmware.c
 * @author Пиминов Никита (nikita.piminoff@yandex.ru)
 * @brief Драйвер VMWare
 * @version 0.3.4
 * @date 2022-11-01
 * @copyright Copyright SayoriOS Team (c) 2022-2023
 */
#include <io/ports.h>

uint8_t vmware_busnum, vmware_slot, vmware_func;

#define VMWARE_VENDORID 0x15AD
#define VMWARE_DEVICEID 0x0405

// Регистры VMware
#define SVGA_REG_ID 0x00
#define SVGA_REG_ENABLE 0x01
#define SVGA_REG_WIDTH 0x02
#define SVGA_REG_HEIGHT 0x03
#define SVGA_REG_MAX_WIDTH 0x04
#define SVGA_REG_MAX_HEIGHT 0x05
#define SVGA_REG_DEPTH 0x06
#define SVGA_REG_BPP 0x07
#define SVGA_REG_PSEUDOCOLOR 0x08
#define SVGA_REG_RED_MASK 0x09
#define SVGA_REG_GREEN_MASK 0x0A
#define SVGA_REG_BLUE_MASK 0x0B
#define SVGA_REG_BYTES_PER_LINE 0x0C
#define SVGA_REG_FB_START 0x0D
#define SVGA_REG_FB_OFFSET 0x0E
#define SVGA_REG_VRAM_SIZE 0x0F
#define SVGA_REG_FB_SIZE 0x10
#define SVGA_REG_CAPABILITIES 0x11
#define SVGA_REG_FIFO_START 0x12
#define SVGA_REG_FIFO_SIZE 0x13

void drv_video_vmware(){
   qemu_log("vmware: init");

    pci_find_device(VMWARE_VENDORID, VMWARE_DEVICEID, &vmware_busnum, &vmware_slot, &vmware_func);
    const uint16_t devnum = pci_get_device(vmware_busnum, vmware_slot, vmware_func);

    qemu_log("VMWARE ID: %d (%x)", devnum, devnum);

    if(devnum == PCI_VENDOR_NO_DEVICE) {
        qemu_log("VMWARE VideoID not connected!");
        return;
    }else{
        qemu_log("Detected VMWARE VideoID");
    }

}