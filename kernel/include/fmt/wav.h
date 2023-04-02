#pragma once

typedef struct {
	char chunkId[4];
	unsigned int chunkSize;
	char format[4];

	char subchunk1Id[4];
	unsigned short audioFormat;
	unsigned short numChannels;

	unsigned int sampleRate;
	unsigned int byteRate;
	unsigned short blockAlign;
	unsigned short bitsPerSample;

	char subchunk2Id[4];
	unsigned int subchunk2Size;
	
} __attribute((packed)) WAV_t;
