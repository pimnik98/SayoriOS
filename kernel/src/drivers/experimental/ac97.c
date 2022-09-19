// AC'97 Driver by NDRAEY

#include <kernel.h>

#include "../../../include/libk/stdbool.h"
#include "../../../include/libk/stdint.h"
#include "../../../include/libk/stdio.h"
#include <io/ports.h>

unsigned int ac97_init() {
	int retval = pci_read(
		pci_get_device(0x8086, 0x2415, -1),
		PCI_BAR0
	);

	qemu_log("[AC'97] Returned value: %d\n", retval);
	return retval;
}
