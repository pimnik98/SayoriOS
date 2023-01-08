/**
 * @file drv/beeper.c
 * @author Пиминов Никита (nikita.piminoff@yandex.ru)
 * @brief Драйвер пищалки
 * @version 0.3.0
 * @date 2022-10-10
 * @copyright Copyright SayoriOS Team (c) 2022
*/

#include <kernel.h>
#include <io/ports.h>
#include <drv/beeper.h>
uint32_t config = 0;            ///< Корректировка

/**
 * @brief [Beeper] Вопроизвести звук
 *
 * @param uint32_t nFrequence - Частота звука
 */
void beeperPlay(uint32_t nFrequence) {
    uint32_t Div;
    uint8_t tmp;
    Div = ((getFrequency()*1000)+config) / (nFrequence);
    outb(0x43, 0xb6);
    outb(0x42, (uint8_t) (Div) );
    outb(0x42, (uint8_t) (Div >> 8));
    tmp = inb(0x61);
    if (tmp != (tmp | 3)) {
        outb(0x61, tmp | 3);
    }
}

/**
 * @brief [Beeper] Выключить звук
 */
void beeperSilent() {
    uint8_t tmp = inb(0x61) & 0xFC;
    outb(0x61, tmp);
}

/**
 * @brief [Beeper] Настроить звук
 *
 * @param uint32_t val - Корректировка частоты
 */
void beeperConfig(uint32_t val){
    config = val;
}

/**
 * @brief [Beeper] Инициализация
 */

void beeperInit(int test){
    qemu_log("[Beeper] Init...");
    beeperPlay(1000);
    sleep(50);
    beeperSilent();
    // FIXME: Beeper interrupts for a short time.
    if (test == 1){
        Note notes[150] = {
            {A4, 200},
            {A4, 200},
            {C5, 200},
            {A4, 200},
            {D5, 200},
            {A4, 200},
            {E5, 200},
            {D5, 200},
            {C5, 200},
            {C5, 200},
            {E5, 200},
            {C5, 200},
            {E5, 200},
            {C5, 200},
            {G5, 200},
            {C5, 200},
            {E5, 200},
            {C5, 200},
            {G4, 200},
            {G4, 200},
            {B4, 200},
            {G4, 200},
            {C5, 200},
            {G4, 200},
            {D5, 200},
            {C5, 200},
            {F4, 200},
            {F4, 200},
            {A4, 200},
            {F4, 200},
            {C5, 200},
            {F4, 200},
            {C5, 200},
            {B4, 200},
            {A4, 200},
            {A4, 200},
            {C5, 200},
            {A4, 200},
            {D5, 200},
            {A4, 200},
            {E5, 200},
            {D5, 200},
            {C5, 200},
            {C5, 200},
            {E5, 200},
            {D5, 200},
            {C5, 200},
            {C5, 200},
            {E5, 200},
            {C5, 200},
            {G5, 200},
            {C5, 200},
            {E5, 200},
            {C5, 200},
            {G4, 200},
            {G4, 200},
            {B4, 200},
            {G4, 200},
            {C5, 200},
            {G4, 200},
            {C5, 200},
            {G4, 200},
            {D5, 200},
            {C5, 200},
            {F4, 200},
            {F4, 200},
            {A4, 200},
            {F4, 200},
            {C5, 200},
            {F4, 200},
            {C5, 200},
            {B4, 200},
            {A4, 200},
            {A4, 200},
            {A4, 200},
            {A4, 200},
            {G4, 200},
            {C5, 200},
            {A4, 200},
            {A4, 200},
            {A4, 200},
            {A4, 200},
            {G4, 200},
            {E4, 200},
            {A4, 200},
            {A4, 200},
            {A4, 200},
            {A4, 200},
            {G4, 200},
            {C5, 200},
            {A4, 200},
            {A4, 200},
            {A4, 200},
            {A4, 200},
            {A4, 200},
            {A4, 200},
            {A4, 200},
            {A4, 200},
            {A4, 200},
            {G4, 200},
            {C4, 200},
            {A4, 200},
            {A4, 200},
            {A4, 200},
            {A4, 200},
            {G4, 200},
            {E4, 200},
            {A4, 200},
            {A4, 200},
            {A4, 200},
            {A4, 200},
            {G4, 200},
            {C5, 200},
            {A4, 200},
            {A4, 200},
            {A4, 200},
            {A4, 200},
            {A4, 200},
            {A4, 200},
            {A4, 200},
            {A4, 200},
        };

        for(int i = 0; i < 122; i++) {
            if(notes[i].freq == 0) continue;
            beeperPlay(notes[i].freq);
            tty_printf("%d ", notes[i].freq);
            if (notes[i].duration == 0){
                beeperSilent();
                sleep(100);
                continue;
            }
            sleep((notes[i].duration)/3);
            beeperSilent();
            sleep(100);
        }
        beeperSilent();
    }
}
