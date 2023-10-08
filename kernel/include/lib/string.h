#pragma once

#include "common.h"

int32_t memcmp(const char *s1, const char *s2, size_t n);
void memcpy(void* destination, const void* source, size_t n);
void* memset(void* ptr, int value, size_t num);
void* memmove(void *dest, void *src, size_t count);

void strver(char *str);
char *strcat(char *s, const char *t);
size_t strspn(const char *s, const char *accept);
int strcpy(char* dest, const char* src);
char *strtok(char *s, const char *delim);
size_t strlen(const char *str);
bool strcmpn(const char *str1, const char *str2);
size_t mb_strlen(const char *str);
void substr(char *dest, const char *source, int from, int length);
int strcmp(const char *s1, const char *s2);
int32_t strncmp(const char *s1, const char *s2, size_t num);

char digit_count(size_t num);
char hex_count(size_t num);
bool isdigit(char a);
size_t itoh(size_t n, char *buffer);
size_t itou(size_t n, char *buffer);
char *strchr(const char *_s, char _c);
size_t struntil(const char* str, const char find);
bool isnumberstr(char* a);

bool isUTF(char c);

#if USE_SSE
void sse_memcpy(void* restrict dest, const void* restrict src, size_t size);
#endif

size_t strcount(const char* string, char character);
