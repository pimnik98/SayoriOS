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

#define PREFERRED_VY 4096
#define PREFERRED_B 32

uint32_t mountPoint;    // Место адресации запросов
uint16_t lfb_resolution_x = 0;
uint16_t lfb_resolution_y = 0;
uint16_t lfb_resolution_b = 0;
uint8_t * lfb_vid_memory = (uint8_t *)0xE0000000;


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
    bgaWriteData(VBE_DISPI_INDEX_ENABLE,0x41);
}

void bgaChangeResize(uint32_t w, uint32_t h, uint32_t bpp){
    bgaWriteData(VBE_DISPI_INDEX_XRES,w);
    bgaWriteData(VBE_DISPI_INDEX_YRES,h);
    bgaWriteData(VBE_DISPI_INDEX_BPP,bpp);
}


static void finalize_graphics(uint16_t x, uint16_t y, uint16_t b) {
	lfb_resolution_x = x;
	lfb_resolution_y = y;
	lfb_resolution_b = b;
    qemu_log("[BGA] [Finale] X:%d Y:%d B:%d Address:%x",x,y,b,lfb_vid_memory);
    bgaDriverInit(x,y,b,mountPoint);
}

uintptr_t lfb_get_address() {
	return (uint8_t)lfb_vid_memory;
}

uintptr_t current_scroll = 0;

void bochs_set_y_offset(uint16_t y) {
    bgaWriteData(VBE_DISPI_INDEX_Y_OFFSET,y);
	current_scroll = y;
}

uint16_t bochs_current_scroll() {
	return current_scroll;
}

void memFound(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t b, uint16_t f){
    finalize_graphics(w,h,b);

	for (uint16_t y = 0; y < h; y++) {
		for (uint16_t x = 0; x < w; x++) {
			uint8_t f = y % 255;
			((uint32_t *)lfb_vid_memory)[x + y * w] = 0xFF000000 | (f * 0x10000) | (f * 0x100) | f;
		}
	}
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
    uint32_t ax = 800;
    uint32_t ay = 600;
    uint32_t ab = PREFERRED_B;
    bgaDisabled();
    bgaChangeResize(ax, ay , ab);
    bgaWriteData(VBE_DISPI_INDEX_VIRT_HEIGHT,PREFERRED_VY);
    bgaEnabled();

    lfb_vid_memory = (uint8_t *)mountPoint;
    finalize_graphics(ax,ay,ab);
    return;

    uint32_t * text_vid_mem = (uint32_t *)0xA0000;
	text_vid_mem[0] = 0xA5ADFACE;

    for (uint8_t fb_offset = 0xE0000000; fb_offset < 0xFF000000; fb_offset += 0x01000000) {
		/* Go find it */
		for (uint8_t x = fb_offset; x < fb_offset + 0xFF0000; x += 0x1000) {
			if (((uint8_t *)x)[0] == 0xA5ADFACE) {
				lfb_vid_memory = (uint8_t *)x;
                finalize_graphics(ax,ay,ab);
                return;
				//goto mem_found;
			}
		}

	}

    //
}
