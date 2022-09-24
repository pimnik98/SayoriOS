#pragma once

#include <string.h>

#define SC_CODE_malloc          4
#define SC_CODE_free            5
#define SC_CODE_realloc         6

void strver(char *str);
int itoa(int n, char *buffer);
void* malloc(int value);
void* calloc(size_t number, size_t size);
void* realloc(void* memory, unsigned int size);
void free(void* memory);

void exit(unsigned int code);
