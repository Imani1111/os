#ifndef STRING_H
#define STRING_H

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;

#include <stddef.h>

void kstrcpy(char* dest, const char* src);
void* kmemcpy(void* dest, const void* src, uint32_t num);
int kstrcmp(uint8_t* str1, uint8_t* str2);
void* kmemset(void* dest, uint8_t val, int count);

#endif
