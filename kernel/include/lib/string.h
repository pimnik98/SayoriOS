#pragma once

int32_t itoa(int32_t n, char *buffer);
void strver(char *str);
void* calloc(size_t nmemb, size_t size);
uint32_t atoi(char s[]);
char *strcat(char *s, const char *t);
size_t strspn(const char *s, const char *accept);
int32_t memcmp(const void *s1, const void *s2, size_t n);
int strcpy(char* dest, char* src);
size_t strlen(const char *str);
size_t mb_strlen(const char *str);
void *memcpy(void *destination, const void *source, size_t n);
void memset(void* ptr, uint8_t value, size_t size);