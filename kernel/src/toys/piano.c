#include <drv/beeper.h>
#include <gui/basics.h>
#include <drv/psf.h>
#include <io/tty.h>
#include <drv/input/keyboard.h>
#include <io/ports.h>

#define PADDING 32
#define KEY_SIZE 32

char keys_black[] = "we tyu o";
char keys_white[] = "asdfghjk";

typedef struct {
    char key;
    uint16_t note;
} KeyboardNote_t;

char keystates[2][8] = {
    {0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0}
};

KeyboardNote_t keyboard_notes[14] = {
    {'w', Cd4},
    {'e', Dd4},
    {'t', Fd4},
    {'y', Gd4},
    {'u', Ad4},
    {'o', Cd5},

    {'a', C4},
    {'s', D4},
    {'d', E4},
    {'f', F4},
    {'g', G4},
    {'h', A4},
    {'j', B4},
    {'k', C5}
};

void draw_key(size_t x, size_t y, char* letter, bool pressed) {
    draw_filled_rectangle(x, y, KEY_SIZE, KEY_SIZE, pressed?0xaaaaaa:0x303030);
    draw_vga_str(letter, 1, x+16, y+16, pressed?0x303030:0xaaaaaa);
}

void play_key(char key) {
    for(char i = 0; i < 14; i++) {
        if(keyboard_notes[i].key == key) {
            beeperPlay(keyboard_notes[i].note);
            break;
        }
    }
}

void draw_piano() {
    size_t dispw = getWidthScreen();
    size_t disph = getHeightScreen();

    size_t starth = (disph - (KEY_SIZE*2)+PADDING)/2;
    size_t startbw = (dispw - (KEY_SIZE+PADDING)*(strlen(keys_black)+1))/2;
    // size_t startww = (dispw - (KEY_SIZE+PADDING)*(strlen(keys_white)+1))/2;

    for(size_t bi = 0; bi < strlen(keys_black); bi++) {
        if(keys_black[bi] == ' ') continue;
        draw_key(PADDING + startbw + (bi*(KEY_SIZE+PADDING)),
                 starth,
                 (char[]){keys_black[bi], 0},
                 keystates[0][bi]
        );
    }

    for(size_t wi = 0; wi < strlen(keys_white); wi++) {
        draw_key(startbw + (wi*(KEY_SIZE+PADDING)),
                 starth+(KEY_SIZE + PADDING),
                 (char[]){keys_white[wi], 0},
                 keystates[1][wi]
        );
    }
}

void handle_key_piano(char key) {
    if(key == 0) return;
    char* printable_key = getCharKeyboard(key, false);
    char pressed = !getPressReleaseKeyboard();

    // qemu_log("Key pressed: %d (%d)", pressed, key);

    if(!pressed) {
        memset(&keystates, 0, 8*2);
        // qemu_log("Key down: %d", key);
        beeperSilent();
    }

    for(size_t i = 0; i < strlen(keys_black); i++) {
        if(printable_key[0] == keys_black[i]) {
            if(pressed) {
                // qemu_log("Key up: %d", key);
                keystates[0][i] = 1;
                play_key(printable_key[0]);
            }
            break;
        }
    }

    for(size_t i = 0; i < strlen(keys_white); i++) {
        if(printable_key[0] == keys_white[i]) {
            if(pressed) {
                // qemu_log("Key up: %d", key);
                keystates[1][i] = 1;
                play_key(printable_key[0]);
            }
            break;
        }
    }
}

void piano() {
    clean_screen();

    set_cursor_enabled(false);
    keyboardctl(KEYBOARD_ECHO, false);

    while(1) {
        char key = getCharRaw();
        if(key == 129 || key == 1) break;

        handle_key_piano(key);

        draw_piano();
        draw_vga_str("NDRAEY (Drew >_) [Press Esc to exit]", 36, 8, getHeightScreen() - 16, 0xaaaaaa);
        punch();
    }

    clean_screen();
    set_cursor_enabled(true);
    keyboardctl(KEYBOARD_ECHO, true);
}