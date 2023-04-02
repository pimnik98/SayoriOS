#include "audio/machinist.hpp"

audio::MachinistServer* __main_machinist_server;

void machinist_server_init() asm("machinist_server_init");
void machinist_server_init() {
    audio::machinist_server_init();
}

void audio::machinist_server_init() {
    __main_machinist_server = new audio::MachinistServer();
}

audio::MachinistServer* audio::machinist_server_get() {
    return __main_machinist_server;
}