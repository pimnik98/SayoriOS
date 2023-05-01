#include "stdio.hpp"
#include "string.hpp"
#include "file.hpp"
#include "conv.hpp"
#include "log.hpp"
#include "vector.hpp"
#include "unique_ptr.hpp"
#include "audio/machinist.hpp"
#include "display.hpp"
#include "gui/window_manager.hpp"

using namespace std;
using namespace audio;

void test_machinist() {
    cout << "Testing Machinist..." << endl;

    // Make new connection to client
    unique_ptr<MachinistClient> client = new MachinistClient();
    client->select_device();

    // Load file data
    File audio("/sound.wav");

    bool error = audio.open();
    
    if(error) {
        cout << "Failed to open file: /sound.wav !!!" << endl;
        return;
    }

    char* data = (char*)memory::alloc(audio.size());
    audio.read_all(data);

    // Set parameters
    client->set_nchannels(2);
    client->set_rate(44100);
    client->set_volume(3, 3);

    // Write data!
    client->write(data, audio.size());

    // Close file
    audio.close();

    // Free memory
    memory::free(data);
}

TTY& operator << (TTY& tty, const std::vector<uint32_t>& obj) {
    for (size_t i = 0, len = obj.get_length() - 1; i < len; i++) {
        tty << (int)obj[i] << (i > (obj.get_length() - 1)?"":", ");
    }

    return tty;
}

void cpp_test() asm("cpp_test");
void cpp_test() {
    // unique_ptr<std::vector<uint32_t>> vec = unique_ptr<std::vector<uint32_t>>(new std::vector<uint32_t>);

    // vec->push_back(1);
    // vec->push_back(2);
    // vec->push_back(3);
    // vec->push_back(4);
    // vec->push_back(5);

    // std::cout << *vec << std::endl;

    // vec->remove(2);

    // std::cout << *vec << std::endl;

    // GUI::WindowManager wm;
    // GUI::Window firstwin;

    // wm.add_window(firstwin);
    // wm.render_once();
}
