#pragma once

#include "common.h"

typedef struct __attribute__((packed)) {
    char chunkID[4];
    uint32_t chunkSize;
    char format[4];
    char subchunk1ID[4];
    uint32_t subchunk1Size;
    uint16_t audioFormat;
    uint16_t numChannels;
    uint32_t sampleRate;
    uint32_t byteRate;
    uint16_t blockAlign;
    uint16_t bitsPerSample;
} WAVHeader;

struct WAVInfoChunk {
	char name[4];
	uint32_t size;
} __attribute__((packed));
