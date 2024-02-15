//
// Created by ndraey on 2/15/24.
//

// PS2 controller

#include "drv/ps2.h"
#include "io/ports.h"

bool ps2_channel1_okay = false;
bool ps2_channel2_okay = false;

uint8_t ps2_read_configuration_byte() {
    ps2_in_wait_until_empty();
    outb(PS2_STATE_REG, 0x20);

    return ps2_read();
}

void ps2_write_configuration_byte(uint8_t byte) {
    ps2_in_wait_until_empty();
    outb(PS2_STATE_REG, 0x60);

    ps2_write(byte);
}

void ps2_in_wait_until_empty() {
    while(inb(PS2_STATE_REG) & (1 << 1));
}

void ps2_out_wait_until_full() {
    while(!(inb(PS2_STATE_REG) & 1));
}

uint8_t ps2_read() {
    ps2_out_wait_until_full();
    return inb(PS2_DATA_PORT);
}

void ps2_write(uint8_t byte) {
    ps2_in_wait_until_empty();
    outb(PS2_DATA_PORT, byte);
}

void ps2_disable_first_device() {
    ps2_in_wait_until_empty();
    outb(PS2_STATE_REG, 0xAD); // 1
}

void ps2_disable_second_device() {
    ps2_in_wait_until_empty();
    outb(PS2_STATE_REG, 0xA7); // 2
}

void ps2_enable_first_device() {
    ps2_in_wait_until_empty();
    outb(PS2_STATE_REG, 0xAE); // 1
}

void ps2_enable_second_device() {
    ps2_in_wait_until_empty();
    outb(PS2_STATE_REG, 0xA8); // 2
}

void ps2_flush() {
    while(inb(PS2_STATE_REG) & 1) {
        inb(PS2_DATA_PORT);
    }
}

// false - error; true - ok
bool ps2_test() {
    ps2_in_wait_until_empty();

    outb(PS2_STATE_REG, 0xAA); // Test

    ps2_out_wait_until_full();

    uint8_t reply = inb(PS2_DATA_PORT);

    return reply == 0x55;
}


void ps2_init() {
    ps2_disable_first_device();
    ps2_disable_second_device();

    ps2_flush();

    ps2_in_wait_until_empty();

    uint8_t conf = ps2_read_configuration_byte();
    ps2_write_configuration_byte(conf & ~(0b1000011));

    // TEST CONTROLLER

    bool test_ok = ps2_test();

    if(test_ok) {
        qemu_ok("PS/2 test ok!");
    } else {
        qemu_ok("PS/2 TEST ERROR!");
        return;
    }

    // Test first port

    ps2_in_wait_until_empty();
    outb(PS2_STATE_REG, 0xAB);

    ps2_out_wait_until_full();
    uint8_t result = inb(PS2_DATA_PORT);

    if(result == 0x00) {
        qemu_ok("Passed test for channel 1!");
        ps2_channel1_okay = true;
    } else {
        qemu_err("Channel 1: Test failed! Result: %x", result);
    }

    // Test second port

    ps2_in_wait_until_empty();
    outb(PS2_STATE_REG, 0xA9);

    ps2_out_wait_until_full();
    result = inb(PS2_DATA_PORT);

    if(result == 0x00) {
        qemu_ok("Passed test for channel 2!");
        ps2_channel2_okay = true;
    } else {
        qemu_err("Channel 2: Test failed! Result: %x", result);
    }

    ps2_enable_first_device();
    ps2_enable_second_device();
}