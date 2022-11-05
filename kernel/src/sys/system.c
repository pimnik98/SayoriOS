#include <kernel.h>

void reboot() {
    qemu_log("REBOOT");

    uint8_t good = 0x02;

    while (good & 0x02)
        good = inb(0x64);
    outb(0x64, 0xFE);
    asm volatile("hlt");
}


void shutdown(){
    qemu_log("SHUTDOWN");
    outw(0xB004, 0x2000);
    outw(0x604, 0x2000);
    outw(0x4004, 0x3400);
}


/**
 * @brief Рисует ASCII Logo
 *
 * @param int mode - Режим COM OR TTY
 */
void drawASCIILogo(int mode){
    if (mode == 0){
qemu_log("\n\n\
 .d8888b.                                      d8b  .d88888b.   .d8888b.    |   SayoriOS v%d.%d.%d\n\
d88P  Y88b                                     Y8P d88P\" \"Y88b d88P  Y88b   |   Code name: %s\n \
Y88b.                                             888     888 Y88b.        |   \n \
 \"Y888b.    8888b.  888  888  .d88b.  888d888 888 888     888  \"Y888b.     |   Assembly time: %s\n \
    \"Y88b.     \"88b 888  888 d88\"\"88b 888P\"   888 888     888     \"Y88b.   |   \n \
      \"888 .d888888 888  888 888  888 888     888 888     888       \"888   |   Links to the project\n \
Y88b  d88P 888  888 Y88b 888 Y88..88P 888     888 Y88b. .d88P Y88b  d88P   |   Git: https://github.com/pimnik98/SayoriOS\n \
 \"Y8888P\"  \"Y888888  \"Y88888  \"Y88P\"  888     888  \"Y88888P\"   \"Y8888P\"    |   VK: https://vk.com/sayorios\n \
                         888                                               |   %s\n \
                    Y8b d88P                                               |   %s\n \
                     \"Y88P\"                                                |   %s\n \
\n",VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH,"Bone",__TIMESTAMP__,"","","");
    } else {
tty_printf("\n\n\
 .d8888b.                                      d8b  .d88888b.   .d8888b.    |   SayoriOS v%d.%d.%d\n\
d88P  Y88b                                     Y8P d88P\" \"Y88b d88P  Y88b   |   Кодовое название: %s\n \
Y88b.                                             888     888 Y88b.        |   \n \
 \"Y888b.    8888b.  888  888  .d88b.  888d888 888 888     888  \"Y888b.     |   Время сборки: %s\n \
    \"Y88b.     \"88b 888  888 d88\"\"88b 888P\"   888 888     888     \"Y88b.   |   \n \
      \"888 .d888888 888  888 888  888 888     888 888     888       \"888   |   Ссылки на проект\n \
Y88b  d88P 888  888 Y88b 888 Y88..88P 888     888 Y88b. .d88P Y88b  d88P   |   Git: https://github.com/pimnik98/SayoriOS\n \
 \"Y8888P\"  \"Y888888  \"Y88888  \"Y88P\"  888     888  \"Y88888P\"   \"Y8888P\"    |   VK: https://vk.com/sayorios\n \
                         888                                               |   %s\n \
                    Y8b d88P                                               |   %s\n \
                     \"Y88P\"                                                |   %s\n \
\n",VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH,"Bone",__TIMESTAMP__,"","","");
    }
}

