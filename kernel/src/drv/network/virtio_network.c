/**
 * @brief Драйвер сетевой карты VirtIO Network
 * @author nikita.piminoff@yandex.ru
 * @version 0.3.5
 * @date 2024-01-01
 * @copyright Copyright SayoriOS Team (c) 2022-2024
 */

#include "../../../include/drv/network/virtio_network.h"
#include <drv/rtl8139.h>
#include <net/cards.h>
#include <drv/pci.h>
#include <io/ports.h>
#include <net/endianess.h>
#include <net/ethernet.h>
#include <debug/hexview.h>
#include <sys/isr.h>
#include "lib/string.h"
#include "mem/vmm.h"
#include "mem/pmm.h"

/*
 *
1. Найдите устройство PCI с поставщиком/устройством 1af4/1000.
2. Получите BAR0 для этого устройства.
3. Запишите 0x01 в BAR0+0x12, чтобы подтвердить, что вы обнаружили устройство.
4. Запишите 0x03 в BAR0+0x12, чтобы уведомить хост о загрузке драйвера.
5. Запишите 0x00000000 в BAR0+0x0E, чтобы выбрать Очередь 0.
6. Запишите 0x00000100 в BAR0+0x08, чтобы установить адрес очереди 0x00100000.
[SKIP??] Запишите 0x00000001 в BAR0+0x0E, чтобы выбрать Очередь 1.
[SKIP??] Запишите 0x00000120 в BAR0+0x08, чтобы установить адрес очереди 0x00120000.
[SKIP??] Настройте цепочки буферов в памяти для обеих очередей.
7. Запишите 0x00010020 в BAR0+0x04, чтобы установить флаги гостевой функции.
8. Запишите 0x07 в BAR0+0x12, чтобы уведомить хост о готовности драйвера.
9. Запишите 0x00 в BAR0+0x10, чтобы уведомить хост о том, что очередь 0 была изменена драйвером.

 */
void vio_ntw_init(){
    qemu_log("[VirtIO] [Network] Init");
    uint8_t busnum, slot, func;
    uint32_t io_base, mem_base, bar_type;
    uint8_t mac[6];
    size_t phys_buffer = 0;
    char* virt_buffer = (char*)0;


    size_t phys_buffer2 = 0;
    char* virt_buffer2 = (char*)0;

    void* transfer_buffer;
    size_t transfer_buffer_phys;


    pci_find_device(VIO_NET_VENDOR, VIO_NET_DEVICE,
                    &busnum, &slot, &func);
    uint16_t devnum = pci_get_device(busnum, slot, func);

    qemu_log("[VirtIO] [Network] ID: %d (%x)", devnum, devnum);

    if(devnum == PCI_VENDOR_NO_DEVICE) {
        qemu_err("[VirtIO] [Network] Error is not connected!");
        return;
    } else {
        qemu_ok("Detected VirtIO Network");
    }

    uint32_t ret = pci_read_confspc_word(busnum, slot, func, 0x10);  // BAR0

    // If bar type is 0 use memory-based access, use port-based otherwise.
    bar_type = ret & 0x1;
    io_base = ret & (~0x3);
    mem_base = ret & (~0xf);

    qemu_log("[VirtIO] [Network] BAR TYPE: %d; IO BASE: %x; MEMORY BASE: %x", bar_type, io_base, mem_base);


    uint32_t mac_part1 = inl(io_base + VIO_NAT_NDR_MAC1P);
    uint16_t mac_part2 = inw(io_base + VIO_NAT_NDR_MAC2P);

    mac[0] = (mac_part1 >> 0) & 0xFF;
    mac[1] = mac_part1 >> 8;
    mac[2] = mac_part1 >> 16;
    mac[3] = mac_part1 >> 24;

    mac[4] = mac_part2 >> 0;
    mac[5] = mac_part2 >> 8;

    qemu_log("Mac is: %v:%v:%v:%v:%v:%v", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    // Создаем буферы

    virt_buffer = kmalloc_common(4, PAGE_SIZE);
    phys_buffer = virt2phys(get_kernel_page_directory(), (virtual_addr_t) virt_buffer);

    virt_buffer2 = kmalloc_common(4, PAGE_SIZE);
    phys_buffer2 = virt2phys(get_kernel_page_directory(), (virtual_addr_t) virt_buffer2);


    transfer_buffer = kmalloc_common(65550, PAGE_SIZE);
    transfer_buffer_phys = virt2phys(get_kernel_page_directory(), (virtual_addr_t)transfer_buffer);

    // 3. Запишите 0x01 в BAR0+0x12, чтобы подтвердить, что вы обнаружили устройство.
    outl(io_base + VIO_NAT_IDR_DS, 0x01);
    // 4. Запишите 0x03 в BAR0+0x12, чтобы уведомить хост о загрузке драйвера.
    outl(io_base + VIO_NAT_IDR_DS, 0x03);
    // 5. Запишите 0x00000000 в BAR0+0x0E, чтобы выбрать Очередь 0.
    outl(io_base + VIO_NAT_IDR_QS, 0x01);
    // 6. Запишите 0x00000100 в BAR0+0x08, чтобы установить адрес очереди 0x00100000.
    outl(io_base + VIO_NAT_IDR_QA, phys_buffer);
    // 7. Запишите 0x00010020 в BAR0+0x04, чтобы установить флаги гостевой функции.
    outl(io_base + VIO_NAT_IDR_GF, 0x00010020);
    // 8. Запишите 0x07 в BAR0+0x12, чтобы уведомить хост о готовности драйвера.
    outl(io_base + VIO_NAT_IDR_DS, 0x01);
    // 9. Запишите 0x00 в BAR0+0x10, чтобы уведомить хост о том, что очередь 0 была изменена драйвером.
    outl(io_base + VIO_NAT_IDR_QN, 0x00);

}