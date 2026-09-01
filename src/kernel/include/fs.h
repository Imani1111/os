#ifndef FS_H
#define FS_H

#include <stddef.h>

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;

#define SUPERBLOCK_SECTOR 10
#define FS_MAGICNUMBER 0xEEEEEEEE
#define DIRECTORY_TABLE_SIZE (64*512)
#define BITMAP_LBA 75

typedef struct {
	uint32_t magic;
	uint32_t total_sectors;
	uint32_t root_dir_lba;
}SuperBlock;

typedef struct {
	char name[32];
	uint32_t start_lba;
	uint32_t size;
	uint32_t attributes;
	uint8_t reserved[20];
}__attribute__((packed)) FileEntry_t;

extern FileEntry_t* root_directory;

void Init_fs(void);
FileEntry_t* find_file(const char* file_name);
FileEntry_t* create_file();


#endif
