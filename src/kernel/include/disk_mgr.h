#ifndef DISKMGR_H
#define DISKMGR_H

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;

#define DATA_PORT 0x1F0
#define FEATURES_REG 0x1F1
#define SECTOR_COUNT_REG 0x1F2
#define LBA_LOW 0x1F3 // Bits 0-7
#define LBA_MID 0x1F4 // Bits 8-15
#define LBA_HIGH 0x1F5 // Bits 16-23
#define DRIVE_REG 0x1F6 // Bits 23-27 of the LBA and some flags
#define COMMAND_PORT 0x1F7

#define MASTER_DRIVE_MODE 0xE0
#define SLAVE_DRIVE_MODE 0xF0
#define READ_FUNC 0x20
#define WRITE_FUNC 0x30

#define FLUSH_CACHE 0xE7

int dskrs1(uint32_t lba, uint8_t* b);
int dskws1(uint32_t lba, uint8_t* b);
int dskrs2(uint32_t lba, uint8_t* b);
int dskws2(uint32_t lba, uint8_t* b);

#endif
