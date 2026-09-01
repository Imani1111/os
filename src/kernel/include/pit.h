#ifndef PIT_H
#define PIT_H

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;

#define PIT_FREQUENCY 1193180

void InitializePIT(uint32_t preferred_freq);
void PITInterruptHandler(void);


#endif
