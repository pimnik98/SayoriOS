#pragma once

int32_t memcmp(const char *s1, const char *s2, size_t n);
void* memcpy(void *destination, const void *source, size_t n);
void* memset(void* ptr, uint8_t value, size_t size);

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

void *memcpy2(void *destination, const void *source, size_t n);
void *memcpy4(void *destination, const void *source, size_t n);

inline void *fastcpy(void *destination, const void *source, size_t n) {
    size_t divisor = (n % 4 == 0 ? 4 : (n % 2 == 0 ? 2 : 1));

    if(divisor == 4) {
        memcpy4(destination, source, n);
    } else if(divisor == 2) {
        memcpy2(destination, source, n);
    } else {
        memcpy(destination, source, n);
    }
}

char digit_count(size_t num);
char hex_count(size_t num);
bool isdigit(char a);