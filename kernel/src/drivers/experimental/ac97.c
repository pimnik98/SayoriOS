// AC'97 Driver by NDRAEY

#include <kernel.h>

#include "../../../include/drivers/experimental/ac97.h"
#include "../../../include/drivers/vfs.h"
#include "../../../include/libk/stdio.h"
#include <io/ports.h>

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

int nam_base;
int nabm_base;

pci_dev_t ac97_dev;

BDL_t buffer_descriptor_list[32] = {0};
unsigned char bdl_index = 0;

void ac97_nam_write_byte(uint8_t field, uint32_t value) {outb(nam_base+field, value);}
void ac97_nam_write_word(uint16_t field, uint32_t value) {outs(nam_base+field, value);}
void ac97_nam_write_dword(uint32_t field, uint32_t value) {outl(nam_base+field, value);}

void ac97_nabm_write_byte(uint8_t field, uint32_t value) {outb(nabm_base+field, value);}
void ac97_nabm_write_word(uint16_t field, uint32_t value) {outs(nabm_base+field, value);}
void ac97_nabm_write_dword(uint32_t field, uint32_t value) {outl(nabm_base+field, value);}

unsigned char ac97_nabm_read_byte(unsigned int field) {return inb(nabm_base+field);}

void ac97_set_bdl(size_t address) {
	ac97_nabm_write_dword(0x10, address);
}

void ac97_set_bdl_default() {
	ac97_nabm_write_dword(0x10, &buffer_descriptor_list);
}

void ac97_debug() {
	qemu_log("[AC'97] NAM: %x", nam_base);
	qemu_log("[AC'97] NABM: %x", nabm_base);
	qemu_log("[AC'97] BDL: ");
	for(int i = 0; i < 32; i++) {
		BDL_t cur = buffer_descriptor_list[i];
		qemu_log("[AC'97]     ADDR: %x LEN: %d META: %d", cur.address, cur.samples, cur.metas);
	}

	qemu_log("Address of BDL: %x", &buffer_descriptor_list);
}

void ac97_init() {
	ac97_dev = pci_get_device(0x8086, 0x2415, -1);
	nam_base = pci_read(ac97_dev, PCI_BAR0);
	nabm_base = pci_read(ac97_dev, PCI_BAR1);

	if(nam_base==0 || nabm_base==0) {
		qemu_log("[AC'97] Driver was not found the device.");
		return;
	}

	qemu_log("[AC'97] NAM: %x; NABM: %x;", nam_base, nabm_base);

	pci_mmio_enable_master(ac97_dev.bus_num, ac97_dev.device_num, ac97_dev.function_num);
	pci_io_enable_master(ac97_dev.bus_num, ac97_dev.device_num, ac97_dev.function_num);

	ac97_nabm_write_dword(GLOBAL_CONTROL_REGISTER, 0x2); // Resume from cold reset
	sleep_ticks(20);
	ac97_nabm_write_byte(0x15, 0x0);

	//ac97_nabm_write_byte(0x0B, 0x2); // Stream reset
	ac97_nabm_write_byte(0x1B, 0x2);
	//ac97_nabm_write_byte(0x2B, 0x2);
	sleep_ticks(20);
	ac97_nam_write_word(0, 0xFF);

	ac97_nam_write_word(0x2, 0); // Max
	ac97_nam_write_word(0x18, 0);

	ac97_set_bdl((size_t)(&buffer_descriptor_list));

	ac97_nam_write_word(0x2A, 0x1);
	ac97_set_sample_rate(48000);
}

void ac97_clear_channels() {
	ac97_nabm_write_byte(0x0B, 0x2); // Stream reset
	ac97_nabm_write_byte(0x1B, 0x2);
	ac97_nabm_write_byte(0x2B, 0x2);
}

void ac97_set_volume(unsigned char left, unsigned char right) {
	if(left>100) left = 100;
	if(right>100) right = 100;
	if(left<0) left = 0;
	if(right<0) right = 0;

	qemu_log("Requested: L: %d R: %d;", left, right);

	if(left==0 && right==0) {
		qemu_log("Mute!");
		ac97_nam_write_word(NAM_PCM_VOL, 0x8000);
		return;
	}
	
	if(left==100 && right==100) {
		qemu_log("Max volume!");
		ac97_nam_write_word(NAM_PCM_VOL, 0);
		return;
	}

	left = (100 - left)*32 / 100;
	right = (100 - right)*32 / 100;

	qemu_log("Converted: L%dR%d;", left, right);
	
	ac97_nam_write_word(NAM_PCM_VOL, (unsigned short)(
		&(struct PCMVol){
			right, 0, left,
			0, 0
		}
	));

	qemu_log("Packed: %d",(unsigned short)(
		&(struct PCMVol){
			right, 0, left,
			0, 0
		}
	));
}

/* From Klaykap/BleskOS
ac97_fill_buffer:
 mov eax, dword [ac97_last_entry]
 shl eax, 3 ;mul 8
 add eax, MEMORY_AC97_BUFFER
 mov ebx, dword [ac97_sound_data]
 mov dword [eax+0], ebx
 mov ebx, 0
 mov bx, word [ac97_sound_length]
 dec bx
 mov dword [eax+4], ebx
 
 ;update pointer in LVI register
 mov eax, dword [ac97_last_entry]
 BASE_OUTB ac97_nabm_base, 0x15, al
 inc dword [ac97_last_entry]
 and dword [ac97_last_entry], 0x1F

 ret
*/

void ac97_do_operate_bdl_once(unsigned int address, unsigned short length) {
	buffer_descriptor_list[bdl_index] = (BDL_t) {
		address,
		length-1,
		0
	};

	ac97_nabm_write_byte(0x15, bdl_index);
	bdl_index = (bdl_index+1)%32;

	qemu_log("Written address: %x; Length: %d; Next: %d;", address, length, bdl_index);
}

void ac97_write_lve() {
	ac97_nabm_write_byte(0x15, bdl_index);
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

void ac97_clear_buffer() {
	for(int i = 0; i < 32; i++) {
		buffer_descriptor_list[i] = (BDL_t){0};
	}

	bdl_index = (ac97_nabm_read_byte(0x14)+1)%32;

	ac97_stop_sound();
}