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

int nam_base;
int nabm_base;

pci_dev_t ac97_dev;

void ac97_init() {
	ac97_dev = pci_get_device(0x8086, 0x2415, -1);
	nam_base = pci_read(ac97_dev, PCI_BAR0);
	nabm_base = pci_read(ac97_dev, PCI_BAR1);

	qemu_log("[AC'97] NAM: %x; NABM: %x;", nam_base, nabm_base);
}

void ac97_nam_write(unsigned int field, unsigned int value) {
	pci_write(ac97_dev, nam_base+field, value);
}

void ac97_nabm_write(unsigned int field, unsigned int value) {
	pci_write(ac97_dev, nabm_base+field, value);
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
