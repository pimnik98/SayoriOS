#include <kernel.h>

#define VBOX_VENDOR_ID 0x80EE
#define VBOX_DEVICE_ID 0xCAFE
#define VBOX_VMMDEV_VERSION 0x00010003
#define VBOX_REQUEST_HEADER_VERSION 0x10001

#define VBOX_REQUEST_GUEST_INFO 50

/* VBox Guest packet header */
struct vbox_header {
        uint32_t size; /* Size of the entire packet (including this header) */
        uint32_t version; /* Version; always VBOX_REQUEST_HEADER_VERSION */
        uint32_t requestType; /* Request code */
        int32_t  rc; /* This will get filled with the return code from the requset */
        uint32_t reserved1; /* These are unused */
        uint32_t reserved2;
};

/* VBox Guest Info packet (legacy) */
struct vbox_guest_info {
        struct vbox_header header;
        uint32_t version;
        uint32_t ostype;
};
static pci_dev_t vbox_pci;
static int vbox_port;
static uint32_t * vbox_vmmdev;

void vbox_guest_init() {
    /* Find the guest device */
    uint32_t VBOX_FOUND = pci_read(pci_get_device(VBOX_VENDOR_ID, VBOX_DEVICE_ID, -1), PCI_BAR0);
    if (VBOX_FOUND == 0){
        qemu_log("[VBox] Error...");
        return;
    }
    qemu_log("[VBox] Found!");
    pci_dev_t vbox_pci = pci_get_device(VBOX_VENDOR_ID, VBOX_DEVICE_ID, PCI_BAR0);

    /* BAR0 is the IO port. */
    vbox_port = pci_read(vbox_pci, PCI_BAR0);

    /* BAR1 is the memory-mapped "vmmdevmem" area. */
    vbox_vmmdev = kheap_malloc(pci_read(vbox_pci, PCI_BAR1));

    /* Allocate some space for our Guest Info packet */
    uint32_t guest_info_phys;
    struct vbox_guest_info * guest_info = kheap_malloc(&guest_info_phys);

    /* Populate the packet */
    guest_info->header.size = sizeof(struct vbox_guest_info);
    guest_info->header.version = VBOX_REQUEST_HEADER_VERSION;
    guest_info->header.requestType = VBOX_REQUEST_GUEST_INFO;
    guest_info->header.rc = 0;
    guest_info->header.reserved1 = 0;
    guest_info->header.reserved2 = 0;
    guest_info->version = VBOX_VMMDEV_VERSION;
    guest_info->ostype = 0; /* 0 = Unknown (32-bit); we don't need to lie about being another OS here */

    /* And send it to the VM */
    outl(vbox_port, guest_info_phys);

    /* (We could check the return value here as well) */
}
