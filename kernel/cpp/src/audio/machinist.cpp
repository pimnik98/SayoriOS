/*  TODO:
    Machinist is an API for interacting kernel and user programs to play audio.

    It should provide simple functions like:
        - Card detection
        - Volume Control
        - Playing from file or stream and recording to file or stream
        - Simple audio effects 
*/

/*
	Machinist's structure is server-client like.
*/

#include "audio/machinist.hpp"
#include "audio/machinist_ac97.hpp"
#include "string.hpp"
#include "log.hpp"

using namespace audio;

MachinistServer::MachinistServer() {
    // Add devices here.

    MachinistDevice ac97_dev = (MachinistDevice) {
        "AC'97",  // Name

        2,  // Max number of channels
        true,  // Fixed channels?

        false,  // Not opened at init

        machinist_ac97_available,
        machinist_ac97_open,
        machinist_ac97_close,
        machinist_ac97_write,
        machinist_ac97_read,
        machinist_ac97_nchannels,
        machinist_ac97_volume,
        machinist_ac97_set_rate
    };

    // AC'97 driver interface
    devices.push_back(ac97_dev);
}

MachinistDevice MachinistServer::get_device(int index) {
    return devices[index];
}

MachinistDevice MachinistServer::get_device(size_t index) {
    return devices[index];
}

MachinistDevice MachinistServer::get_device(char* name) {
    for (size_t i = 0, len = devices.get_length(); i < len; i++) {
        MachinistDevice element = devices[i];

        if(strcmp(element.name, name) == 0) {
            return element;
        }
    }
}

void MachinistServer::open(MachinistDevice& dev) {
    if(dev.is_opened)
        return;

    dev.open();
    
    dev.is_opened = true;
}

void MachinistServer::close(MachinistDevice& dev) {
    if(!dev.is_opened)
        return;

    dev.close();

    dev.is_opened = false;
}

void MachinistServer::write(MachinistDevice& dev, char* data, size_t length) {
    if(!dev.is_opened)
        return;

    dev.write(data, length);
}

void MachinistServer::read(MachinistDevice& dev, char* buffer, size_t length) {
    if(!dev.is_opened)
        return;

    dev.read(buffer, length);
}

MachinistServer::~MachinistServer() {
    for (size_t i = 0, len = devices.get_length(); i < len; i++) {
        auto dev = devices[i];

        close(dev);
    }
}