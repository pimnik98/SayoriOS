#include "audio/machinist.hpp"
#include "log.hpp"

using namespace audio;

MachinistClient::MachinistClient() {
    std::qemu_log("Client initialized.\n");
    
    connection = machinist_server_get();

    if(connection == nullptr)
        return;
}

void MachinistClient::select_device() {
    device = connection->get_device(0);
}

void MachinistClient::select_device(int index) {
    device = connection->get_device(index);
}

void MachinistClient::open() {
    device.open();
}

void MachinistClient::write(void* data, size_t length) {
    device.write((char*)data, length);
}

void MachinistClient::read(void* buffer, size_t length) {
    device.read((char*)buffer, length);
}

void MachinistClient::set_nchannels(uint8_t chans) {
    device.set_nchannels(chans);
}

void MachinistClient::set_rate(uint32_t rate) {
    device.set_rate(rate);
}

void MachinistClient::set_volume(uint8_t left, uint8_t right) {
    device.set_volume(left, right);
}

void MachinistClient::close() {
    device.close();
}

MachinistClient::~MachinistClient() {
    std::qemu_log("Client destroyed.\n");
}