#pragma once

#include "stdint.h"
#include "stdbool.h"

bool isUTF(char c);
bool isSymbol(char c);
void setColorFont(uint32_t color);
uint32_t getColorFont();
uint32_t getConfigFonts(int k);
void setConfigurationFont(uint32_t x,uint32_t p,uint32_t h);
uint32_t SymConvert(char c,char c1,char c2);
uint32_t UTFConvert(char c,char c1);
void setFontPath(char* path,char* dat);
void loadFontData();
void fontInit();
void destroyFont();
uint32_t getPositionChar(uint32_t c,uint32_t offset);
char drawFont(uint32_t x, uint32_t y, uint32_t sx, uint32_t sy, uint32_t width, uint32_t height);
void drawStringFont(char str[],int kx,int ky, int py);
