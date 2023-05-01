#pragma once

#define MOUSE_RESET           0xFF
#define MOUSE_RESEND          0xFE
#define MOUSE_DEFAULTS        0xF6
#define MOUSE_STREAMING_OFF   0xF5
#define MOUSE_STREAMING_ON    0xF4
#define MOUSE_SET_SAMPLE_RATE 0xF3
#define MOUSE_GET_ID          0xF2
#define MOUSE_REQUEST_PACKET  0xEB
#define MOUSE_REQUEST_STATUS  0xE9
#define MOUSE_SET_RESOLUTION  0xE8
#define MOUSE_CONTROL_PORT    0x64
#define MOUSE_CMD_SEND        0xD4
#define MOUSE_DATA_PORT       0x60

// Mouse answers
#define MOUSE_ACK             0xFA

extern int mouse_wheel; // change to 0 after each reading

void mouse_set_show_system_cursor(bool set);
bool mouse_get_show_system_cursor();

uint32_t mouse_get_x();
uint32_t mouse_get_y();
uint8_t  mouse_get_b1();
uint8_t  mouse_get_b2();
uint8_t  mouse_get_b3();
uint8_t  mouse_get_b4();
uint8_t  mouse_get_b5();
