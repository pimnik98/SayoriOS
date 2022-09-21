// AC'97 Driver by NDRAEY

#include <kernel.h>

#include "../../../include/libk/stdbool.h"
#include "../../../include/libk/stdint.h"
#include "../../../include/libk/stdio.h"
#include <io/ports.h>

#define NAM_RESET 0x00
#define NAM_MASTER_VOL 0x02
#define NAM_MIC_VOL 0x0E
#define NAM_PCM_VOL 0x18
#define NAM_SELECT_INPUT 0x1A
#define NAM_SAMPLE_RATE_FRONT 0x2C

/*
0x00	NABM register box for PCM IN	below
0x10	NABM register box for PCM OUT	below
0x20	NABM register box for Microphone	below
0x2C	Global Control Register	dword
0x30	Global Status Register	dword
*/

/*
0x00	Physical Address of Buffer Descriptor List	dword
0x04	Number of Actual Processed Buffer Descriptor Entry	byte
0x05	Number of all Descriptor Entries	byte
0x06	Status of transferring Data	word
0x08	Number of transferred Samples in Actual Processed Entry	word
0x0A	Number of next processed Buffer Entry	byte
0x0B	Transfer Control	byte
*/


#define GLOBAL_CONTROL_REGISTER 0x2C
#define GLOBAL_STATUS_REGISTER 0x2C

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
