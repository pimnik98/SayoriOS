#pragma once

#include <common.h>
/*
#define uint8_t unsigned char
#define uint16_t unsigned short
*/

typedef struct sayori_time {
	uint8_t seconds;
    uint8_t minutes;
    uint8_t hours;
    uint8_t day;
    uint8_t month;
    uint16_t year;
    uint8_t century;
} sayori_time_t;

int isleap(int year);
struct sayori_time get_time();
