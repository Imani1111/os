#ifndef HEAP_H
#define HEAP_H

#include <stddef.h>

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;
typedef signed int int32_t;

#define SBRK_FAIL ((void*)-1)

typedef struct HeapBlock{
	uint32_t size;
	uint8_t is_free;
	struct HeapBlock* next;
	struct HeapBlock* prev;
}HeapBlockMetaData;

void* my_malloc(uint32_t size_in_bytes);
void my_free(void* ptr);
void* sbrk(int32_t increment);
#endif
