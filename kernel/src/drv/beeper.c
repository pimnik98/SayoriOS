/**
 * @file drv/beeper.c
 * @author Пиминов Никита (nikita.piminoff@yandex.ru)
 * @brief Драйвер пищалки
 * @version 0.3.4
 * @date 2022-10-10
 * @copyright Copyright SayoriOS Team (c) 2022-2023
*/

#include <io/ports.h>
#include <drv/beeper.h>
#include <sys/timer.h>

uint32_t config = 0;            ///< Корректировка

/**
 * @brief Вопроизвести звук (квадратной формы волны)
 *
 * @param frequency - Частота звука
 */
void beeperPlay(uint32_t frequency) {
    uint32_t Div;
    uint8_t tmp;

    Div = getFrequency() * 1000;
    Div /= frequency;
    
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