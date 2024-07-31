// AC'97 driver by NDRAEY

// FIXME: Not working in VirtualBox, only in QEMU

#pragma once

#include <lib/math.h>
#include <common.h>

#define AC97_VENDOR 0x8086
#define AC97_DEVICE 0x2415

typedef struct {
    uint32_t reserved : 19;
    uint8_t channel : 2;
    uint8_t sample : 2;
} ac97_global_status_t;

#define AC97_MIN_RATE 8000
#define AC97_MAX_RATE 48000

#define NAM_RESET 0x00
#define NAM_SET_MASTER_VOLUME 0x02
#define NAM_SET_PCM_VOLUME 0x18
#define NAM_EXTENDED_ID 0x28
#define NAM_EXTENDED_AUDIO 0x2A
#define NAM_SAMPLE_RATE 0x2C

#define NABM_PCM_OUT 0x10
#define NABM_GLOBAL_CONTROL 0x2C
#define NABM_GLOBAL_STATUS  0x30

typedef struct {
    void* memory_pos;
    uint16_t sample_count;
    uint16_t flags;
} AC97_BDL_t;

// Volume in dB, not %
void ac97_set_master_volume(uint8_t left, uint8_t right, bool mute);

// Volume in dB, not %
void ac97_set_pcm_volume(uint8_t right, uint8_t left, bool mute);
void ac97_set_pcm_sample_rate(uint16_t sample_rate);
void ac97_reset_channel();
void ac97_clear_status_register();
void ac97_update_bdl();
void ac97_update_lvi(uint8_t index);
void ac97_set_play_sound(bool play);
void ac97_init();
void ac97_FillBDLs();
void ac97_WriteAll(void* buffer, size_t size);
bool ac97_is_initialized();
