// AC'97 driver by NDRAEY (Drew Pavlenko >_)

// FIXME: Not working in VirtualBox, only in QEMU

#pragma once

#include <lib/math.h>
#include <io/ports.h>
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

#define BUFFER_SIZE (64 * 1024)

typedef struct {
    uint32_t memory_pos;
    uint16_t sample_count;
    uint16_t flags;
} __attribute__((packed)) AC97_BDL_t;

static AC97_BDL_t ac97_bdl_buffer[32];
static uint16_t native_audio_mixer = 0;
static uint16_t native_audio_bus_master = 0;

#define ac97_clear_status_register() outb(native_audio_bus_master + 0x16, 0x1C)
#define _ac97_update_bdl(address) outl(native_audio_bus_master + NABM_PCM_OUT, address)
#define ac97_update_bdl() _ac97_update_bdl((uint32_t)&ac97_bdl_buffer)
#define ac97_update_lvi(index) outb(native_audio_bus_master + NABM_PCM_OUT + 0x05, (uint8_t)(index))
#define ac97_clear_bdl() memset(&ac97_bdl_buffer, 0, sizeof(AC97_BDL_t)*31)

// Volume in dB, not %
void ac97_set_master_volume(uint8_t left, uint8_t right, bool mute);

// Volume in dB, not %
void ac97_set_pcm_volume(uint8_t right, uint8_t left, bool mute);
void ac97_set_pcm_sample_rate(uint32_t sample_rate);
void ac97_load_data(char* data, uint32_t length);
void ac97_reset_channel();
void ac97_init();
size_t ac97_copy_user_memory_to_dma(char* data, size_t length);
void ac97_single_page_write(ssize_t page_num);
void ac97_single_page_write_wait(ssize_t page_num);
void ac97_destroy_user_buffer();
bool ac97_is_initialized();

static inline void ac97_set_play_sound(bool play) {
    outb(native_audio_bus_master + 0x1b, inb(native_audio_bus_master + 0x1B) | play);
}
