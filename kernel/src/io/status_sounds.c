#include <io/ports.h>
#include <drv/beeper.h>
#include <io/status_sounds.h>
#include "sys/timer.h"

void ERROR_sound()
{
    beeperPlay(500);
    sleep_ms(50);
    beeperSilent();
    beeperPlay(250);
    sleep_ms(50);
    beeperSilent();
}

void ATTENTION_sound()
{
    beeperPlay(1000);
    sleep_ms(50);
    beeperSilent();
    beeperPlay(1000);
    sleep_ms(50);
    beeperSilent();
    beeperPlay(1000);
    sleep_ms(50);
    beeperSilent();
}

void ALERT_sound()
{
    beeperPlay(250);
    sleep_ms(50);
    beeperSilent();
    beeperPlay(500);
    sleep_ms(50);
    beeperSilent();
}

void GLOBAL_ERROR_sound()
{
    beeperPlay(500);
    sleep_ms(100);
    beeperSilent();
    beeperPlay(250);
    sleep_ms(100);
    beeperSilent();
    beeperPlay(100);
    sleep_ms(100);
    beeperSilent();
    beeperPlay(50);
    sleep_ms(200);
    beeperSilent();
}
