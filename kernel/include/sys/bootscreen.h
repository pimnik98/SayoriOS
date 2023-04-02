#pragma once

#include <common.h>

void bootScreenLazy(bool l);
void bootScreenChangeTheme(uint32_t th);
uint32_t bootScreenTheme(uint32_t type);
void bootScreenClose(uint32_t bg, uint32_t tx);
void bootScreenChangeMode(int m);
void bootScreenInfo();
void bootScreenProcentPaint();
void bootScreenPaint(char* title);
void bootScreenInit(uint32_t count);