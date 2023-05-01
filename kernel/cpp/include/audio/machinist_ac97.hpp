#pragma once

#include "stdint.hpp"

bool machinist_ac97_available();
void machinist_ac97_open();
void machinist_ac97_close();
void machinist_ac97_write(char* data, size_t length);

// TODO: Implement read function for AC97 codec
void machinist_ac97_read(char* buffer, size_t length);

// Ignored, because AC97 has no support for varibale channel count
void machinist_ac97_nchannels(uint8_t chans);
void machinist_ac97_volume(uint8_t left, uint8_t right);
void machinist_ac97_set_rate(uint32_t rate);