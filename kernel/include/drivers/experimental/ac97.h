// AC'97 Driver by NDRAEY
#pragma once

#include <kernel.h>

#include "../../../include/libk/stdbool.h"
#include "../../../include/libk/stdint.h"

#define NAM_RESET 0x00
#define NAM_MASTER_VOL 0x02
#define NAM_MIC_VOL 0x0E
#define NAM_PCM_VOL 0x18
#define NAM_SELECT_INPUT 0x1A
#define NAM_SAMPLE_RATE_FRONT 0x2C

#define GLOBAL_CONTROL_REGISTER 0x2C
#define GLOBAL_STATUS_REGISTER 0x30

#define AC97_BDE_MAX_LENGTH 0xFFFE

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
	unsigned int right : 5;
	unsigned int zero : 3;
	unsigned int left : 5;
	unsigned int zero2 : 2;
	unsigned int mute : 1;
};

struct PCMVol_rev {
	unsigned int mute : 1;
	unsigned int zero2 : 1;
	unsigned int left : 5;
	unsigned int zero : 3;
	unsigned int right : 5;
};

typedef struct buffer_descriptor_entry {
	unsigned int address;
	unsigned short samples;
	unsigned short metas;
} BDL_t;

void ac97_nam_write_byte(unsigned char field, unsigned int value);
void ac97_nam_write_word(unsigned short field, unsigned int value);
void ac97_nam_write_dword(unsigned int field, unsigned int value);

void ac97_nabm_write_byte(unsigned char field, unsigned int value);
void ac97_nabm_write_word(unsigned short field, unsigned int value);
void ac97_nabm_write_dword(unsigned int field, unsigned int value);

unsigned int ac97_read(unsigned int field);
void ac97_set_bdl(unsigned int address);
void ac97_init();
void ac97_set_volume(unsigned char left, unsigned char right);
void ac97_do_operate_bdl_once(unsigned int address, unsigned short length);
void ac97_set_sample_rate(unsigned short rate);

void ac97_play_sound();
void ac97_stop_sound();