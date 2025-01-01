/*    Драйвер SoundBlaster 16
 *
 *    By Drew Pavlenko aka NDRAEY
 */ 

// Нам нужно место в пределах 0x00100000 - 0x00FFFFFF
// SoundBlaster 16 работает только с ними.
// Возьмем где-нибудь адрес с длиной 4096 байт

#define LOAD        0x2A0000
#define LOAD_LENGTH 4096

#include <drv/sb16.h>
#include <lib/string.h>
#include <sys/memory.h>

char can_play_audio = 0;
char* driver_memory = (char*)LOAD;

char sb16_init() {
	char audio = sb16_dsp_reset();
	if(audio==0xFFFFFFFF) {
		qemu_log("SB16: Device not found!!!");
		return 0;
	}else{
		qemu_log("SB16: Allocating at: %x\n", LOAD);
		physaddr_t paddr = alloc_phys_pages(1);
		if (!paddr) return 0;

		qemu_log("Mapping...");
		map_pages(get_kernel_dir(),
			LOAD,
			paddr,
			PAGE_SIZE,
			0x07
		);
		can_play_audio = 1;
		return 1;
	}
}

void sb16_dsp_write(char cmd) {
	while(inb(DSP_WRITE) & 128);
	outb(DSP_WRITE, cmd);
}

char sb16_can_play_audio() {
	return can_play_audio;
}

char sb16_dsp_reset() {
	outb(DSP_RESET, 1);
	for(char _ = 0; _<64; _++) {asm volatile("nop");}
	outb(DSP_RESET, 0);

	return inb(DSP_READ);
}

void sb16_set_irq(char irq) {
	outb(DSP_MIXER_PORT, 0x80);
	outb(DSP_MIXER_DATA, irq);
}

void sb16_turn_speaker_on() {
	sb16_dsp_write(0xD1);
}

void sb16_program_dma16(int address, short length) {
	outb(0xD4, 5); // Channel
	outb(0xD8, 1); // Flip-flop
	outb(0xD6, 0x58 + 1); // 16-bit sound, no FIFO
	char page = (address&0xFF0000)>>16;  // 0x[AA]BBCC
	char lop  = address&0x0000FF;        // 0xAABB[CC]
	char hip  = (address&0x00FF00)>>8;   // 0xAA[BB]CC
	outb(0x8B, page);
	outb(0xC4, lop);
	outb(0xC4, hip);
	char lol = (length&0x00FF);
	char hil = (length&0xFF00)>>8;
	outb(0xC6, lol);
	outb(0xC6, hil);
	outb(0xD4, 1);
}

unsigned int sb16_calculate_time_constant(char channels, int sampling_rate) {
	return 65536-(256000000/(channels*sampling_rate));
}

void sb16_program(unsigned int sampling_rate, char stereo, char eightbit, char sign, short length) {
	length--;
	/*
	outb(DSP_WRITE, 0x40);
	outb(DSP_WRITE, sb16_calculate_time_constant(channels, sampling_rate));
	outb(DSP_WRITE, eightbit?0xC0:0xB0);
	outb(DSP_WRITE, (channels>=1?0b00100000:0)|(sign>=1?0b00010000:0));
	outb(DSP_WRITE, length&0x00FF);
	outb(DSP_WRITE, (length&0xFF00)>>8);
	*/

	sb16_dsp_write(0x40);
	// sb16_dsp_write(sb16_calculate_time_constant(channels, sampling_rate));
	sb16_dsp_write(145);
	sb16_dsp_write(eightbit?0xC0:0xB0);
	sb16_dsp_write((stereo?0b00100000:0)|(sign?0b00010000:0));
	sb16_dsp_write(length&0x00FF);
	sb16_dsp_write((length&0xFF00)>>8);
}

void sb16_set_master_volume(char left, char right) {
	outb(DSP_MIXER_PORT, DSP_M_MAST_VOLU);
	outb(DSP_MIXER_DATA, right|(left<<4));
}

void sb16_play_audio(char *data, unsigned int sampling_rate, char channels, char eightbit, char sign, int length) {
	// 1. Reset DSP
	qemu_log("DSP Reset");
	sb16_dsp_reset();

	// 2. Load sound data to memory
	qemu_log("Loaded %d bytes to driver memory", LOAD_LENGTH);
	memcpy(driver_memory, data, LOAD_LENGTH);

	// 3. Set master volume
	qemu_log("Set volume");
	sb16_set_master_volume(0xA, 0xA);
	// 4. Turn speaker on
	qemu_log("Speakers: on");
	sb16_turn_speaker_on();
	// 6, 7, 8, 9, 10
	qemu_log("Programming...");
	sb16_program(sampling_rate, channels, eightbit, sign, LOAD_LENGTH);

	tty_printf("DAT1: %d\nDAT2: %d\n", driver_memory[1], data[1]);

	sb16_program_dma16(LOAD, LOAD_LENGTH);
	qemu_log("Finished");

	/*while(loaded<length-1) {
		memcpy(driver_memory, data, LOAD_LENGTH);
		sb16_program_dma16(channels, driver_memory, LOAD_LENGTH);
		loaded+=LOAD_LENGTH;
		data+=LOAD_LENGTH;
	}*/
}
