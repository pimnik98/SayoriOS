// AC'97 Driver by NDRAEY

#include <kernel.h>

#include "../../../include/libk/stdbool.h"
#include "../../../include/libk/stdint.h"
#include "../../../include/libk/stdio.h"
#include <io/ports.h>

#define RESET 0x00
#define MASTER_VOL 0x02
#define MIC_VOL 0x0E
#define PCM_VOL 0x18
#define SELECT_INPUT 0x1A
#define SAMPLE_RATE_FRONT 0x2C

struct MasterVol {
	unsigned int right : 5;
	unsigned int zero : 1;
	unsigned int left : 5;
	unsigned int zero2 : 1;
	unsigned int mute : 1;
};

struct PCMVol {
	unsigned int right : 4;
	unsigned int zero : 2;
	unsigned int left : 4;
	unsigned int zero2 : 1;
	unsigned int mute : 1;
};

unsigned int ac97_init() {
	int retval = pci_read(
		pci_get_device(0x8086, 0x2415, -1),
		PCI_BAR0
	);

	qemu_log("[AC'97] Returned value: %d\n", retval);
	return retval;
}

void ac97_test() {
	struct MasterVol mast = { // Half of volume
		0b111010,
		0,
		0b111010,
		0,
		0
	};

	struct PCMVol pcm = { // Half of volume
		16,
		0,
		16,
		0,
		0
	};

	// XXX: NDRAEY NEEDS HELP (Minimal code to play the audio)
}
