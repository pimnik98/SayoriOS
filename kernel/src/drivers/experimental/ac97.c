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
#define GLOBAL_STATUS_REGISTER 0x30

struct MasterVol {
	unsigned int right : 5;
	unsigned int zero : 1;
	unsigned int left : 5;
	unsigned int zero2 : 1;
	unsigned int mute : 1;
};

struct MasterVol_rev {
	unsigned int mute : 1;
	unsigned int zero2 : 1;
	unsigned int left : 5;
	unsigned int zero : 1;
	unsigned int right : 5;
};

struct PCMVol {
	unsigned int right : 4;
	unsigned int zero : 2;
	unsigned int left : 4;
	unsigned int zero2 : 1;
	unsigned int mute : 1;
};

struct PCMVol_rev {
	unsigned int mute : 1;
	unsigned int zero2 : 1;
	unsigned int left : 4;
	unsigned int zero : 2;
	unsigned int right : 4;
};

int nam_base;
int nabm_base;

pci_dev_t ac97_dev;

void ac97_nam_write_byte(unsigned char field, unsigned int value) {outb(nam_base+field, value);}
void ac97_nam_write_word(unsigned short field, unsigned int value) {outs(nam_base+field, value);}
void ac97_nam_write_dword(unsigned int field, unsigned int value) {outl(nam_base+field, value);}

void ac97_nabm_write_byte(unsigned char field, unsigned int value) {outb(nabm_base+field, value);}
void ac97_nabm_write_word(unsigned short field, unsigned int value) {outs(nabm_base+field, value);}
void ac97_nabm_write_dword(unsigned int field, unsigned int value) {outl(nabm_base+field, value);}

unsigned int ac97_read(unsigned int field) {
	return pci_read(ac97_dev, field);
}

void ac97_set_memory(unsigned int address) {
	ac97_nabm_write_dword(0x10, address);
}

void ac97_init(unsigned int memory) {
	ac97_dev = pci_get_device(0x8086, 0x2415, -1);
	nam_base = pci_read(ac97_dev, PCI_BAR0);
	nabm_base = pci_read(ac97_dev, PCI_BAR1);

	if(nam_base==0 || nabm_base==0) {
		qemu_log("[AC'97] Driver was not found the device.");
		return;
	}

	qemu_log("[AC'97] NAM: %x; NABM: %x;", nam_base, nabm_base);

	ac97_nabm_write_dword(GLOBAL_CONTROL_REGISTER, 0x2); // Resume from cold reset
	for(unsigned i = 0; i<20; i++) {
		// Sleep
	}
	ac97_nabm_write_byte(0x0B, 0x2); // Stream reset
	ac97_nabm_write_byte(0x1B, 0x2);
	ac97_nabm_write_byte(0x2B, 0x2);
	for(unsigned i = 0; i<20; i++) {
		// Sleep
	}
	ac97_nabm_write_byte(0x15, 0x0);
	ac97_nam_write_word(NAM_RESET, 0xFF);

	ac97_nam_write_word(NAM_MASTER_VOL, 0); // Max
	ac97_nam_write_word(NAM_PCM_VOL, 0);

	ac97_set_memory(memory);
}

void ac97_set_master_volume(unsigned char left, unsigned char right) {
	if(left>100) left = 100;
	if(right>100) right = 100;
	if(left<0) left = 0;
	if(right<0) right = 0;

	left = 100 - left;
	right = 100 - right;

	left /= 31;
	right /= 31;

	if(left==0 && right==0) {
		ac97_nam_write_word(NAM_PCM_VOL, (unsigned short)(
			&(struct PCMVol_rev){
				1,
				0,
				left,
				0,
				right
			}
		));
		return;
	}

	ac97_nam_write_word(NAM_PCM_VOL, (unsigned short)(
		&(struct PCMVol_rev){
			0,
			0,
			left,
			0,
			right
		}
	));
}

void ac97_set_sample_rate(unsigned short rate) {
	ac97_nam_write_word(0x2C, rate);
	ac97_nam_write_word(0x2E, rate);
	ac97_nam_write_word(0x30, rate);
	ac97_nam_write_word(0x32, rate);
}

void ac97_play_sound() {
	ac97_nabm_write_byte(0x1B, 0x1);
	ac97_nabm_write_word(0x16, 0x1C);
}

void ac97_stop_sound() {
	ac97_nabm_write_byte(0x1B, 0x0);
	ac97_nabm_write_word(0x16, 0x1C);
}

void ac97_test() {
	// XXX: NDRAEY NEEDS HELP (Minimal code to play the audio)
}
