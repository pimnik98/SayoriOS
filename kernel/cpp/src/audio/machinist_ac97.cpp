#include "stdint.hpp"

extern "C" {
    // Volume in dB
    void ac97_set_master_volume(uint8_t left, uint8_t right, bool mute);
    void ac97_set_pcm_volume(uint8_t right, uint8_t left, bool mute);

    // Sample rate in Hz
    void ac97_set_pcm_sample_rate(uint32_t sample_rate);
    
    // Data
    size_t ac97_copy_user_memory_to_dma(char* data, size_t length);
    void ac97_single_page_write_wait(ssize_t page_num);
    void ac97_load_data(char* data, uint32_t length);

    void ac97_reset_channel();
    void ac97_set_play_sound(bool play);

    void ac97_destroy_user_buffer();
    bool ac97_is_initialized();
};

bool machinist_ac97_available() {
    return ac97_is_initialized();
}

void machinist_ac97_open() {}

void machinist_ac97_close() {
    ac97_reset_channel();
    ac97_destroy_user_buffer();
}

void machinist_ac97_write(char* data, size_t length) {
    size_t page_count = ac97_copy_user_memory_to_dma(data, length);

    for(ssize_t i = page_count; i > 0; i-= 32) {
        ac97_single_page_write_wait(i);
    }

    ac97_reset_channel();
}

// TODO: Implement read function for AC97 codec
void machinist_ac97_read(char* buffer, size_t length) {}

// Ignored, because AC97 has no support for varibale channel count
void machinist_ac97_nchannels(uint8_t chans) {}

void machinist_ac97_volume(uint8_t left, uint8_t right) {
    bool mute = (left + right) == 0;

    ac97_set_master_volume(left, right, mute);
    ac97_set_pcm_volume(left, right, mute);
}

void machinist_ac97_set_rate(uint32_t rate) {
    ac97_set_pcm_sample_rate(rate);
}