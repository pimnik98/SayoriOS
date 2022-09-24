#pragma once

#include "stdint.h"
#include "stdbool.h"

char* fileFont = "/initrd/var/fonts/MicrosoftLuciaConsole18.duke";
char* datFont = "/initrd/var/fonts/MicrosoftLuciaConsole.fdat";
struct DukeImageMeta* fontMeta;
uint32_t err = -1, xF = 0, pF = 0, hF = 0, colorFont = 0xFFFFFF, mW, mH, mA;
char* imageFont;
char alphaFont;
char* configFont;
char*** array;
char* Alphabet = "АБВГДЕЁЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯйцукенгшщзхъфывапролджэячсмитьбюё!«№;%:?*()_+-=@#$^&[]{}|\\/QWERTYUIOPASDFGHJKLZXCVBNMqwertyuiopasdfghjklzxcvbnm1234567890.,";

uint32_t Map[250] = {160,159,158,157,156,155,175,154,153,152,151,150,149,148,147,146,145,144,143,142,141,140,139,138,137,136,135,134,133,132,131,130,129,119,169,172,118,123,115,125,167,166,121,170,165,171,164,126,128,113,175,114,117,124,122,162,160,168,174,116,120,173,163,127,161,158,33,1147,3278,1988,37,58,63,42,40,41,95,43,45,61,64,35,36,94,38,91,93,123,125,124,92,47,81,87,69,82,84,89,85,73,79,80,65,83,68,70,71,72,74,75,76,90,88,67,86,66,78,77,113,119,101,114,116,121,117,105,111,112,97,115,100,102,103,104,106,107,108,122,120,99,118,98,110,109,49,50,51,52,53,54,55,56,57,48,46,44,1144,1151,1149,1142,3238,-68,3236,-81,3242,-96,3242,-95,3242,-86,3242,-85,3242,-84,3242,-78,3242,-96,3242,-95,3242,-78,3242,-70,3242,-68,3240,-124,3240,-118,3240,-117,3240,-116,3240,-113,3240,-90,3238,-70,3238,-69,3270,-98,1149,-61,-105,3278,-123,3278,-109,3278,2166,-124,-94,-50,-87,75,-61,-123,3278,-82,3276,-109,3276,-108,3276,-101,3276,-100,3276,-99,3276,-98,3276,-115,3276,-114,3270,-126,3270,-122,3270,-113,3270,-111,3270,-110,3270,-107,3270,-103,3270,-102,3270,-98,3270,-97,3270,-87,3270,-85,3268,-120,3268,-96,3268,-95,3268,-92,3268,-91,1127,1157,1147,1131,3286,-71,3286,-70,34,3286,-104,3286,-103,3286,-100,3286,-99,3286,-102,3286,-98,1151,1136,3286,-96,3286,-95,3286,-90,3278,-94,1149,1144,3282,-84,3282,-93,1153,1155,3282,-67,1156,1154,3242,-124,3242,-128};

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
void drawCharFont(char c,char c1,int x,int y);
void drawStringFont(char str[],int kx,int ky, int py);
