#pragma once

#define KEYBOARD_ECHO 1

char* __getCharKeyboard(char* en_s,char* en_b,char* ru_s,char* ru_b);
char* getCharKeyboard(int key, bool mode);
void keyboardHandler(registers_t regs);
void keyboardctl(uint8_t param, bool value);
int getCharRaw();
void* getCharKeyboardWait(bool use_int);
void keyboardInit();
void gets(char *buffer);
bool is_lctrl_key();
int getCharRaw();
int getIntKeyboardWait();
uint8_t getPressReleaseKeyboard();
int gets_max(char *buffer, int length);