#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define TABLE_SIZE (64*512)
#define SYSTEM_FILE 0x01


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
}FileEntry_t;

int main()
{
	uint8_t sector[512] = {0};

	SuperBlock* sb = (SuperBlock*)sector;
	sb->magic = 0xEEEEEEEE;
	sb->total_sectors = 2048;
	sb->root_dir_lba = 11;

	FILE* f = fopen("superblock.bin", "wb");
	fwrite(sector, 1, 512, f);
	fclose(f);

	printf("os.fs ==>> Superblock generated successfully\n");
		
	uint8_t directory_table[TABLE_SIZE] = {0};

	FileEntry_t* entries = (FileEntry_t*)directory_table;
	strcpy(entries[0].name, "kernel.bin");
	entries[0].start_lba = 75;
	entries[0].size = 4096;
	entries[0].attributes = SYSTEM_FILE;

	FILE* d = fopen("directory.bin", "wb");
	fwrite(directory_table, 1, TABLE_SIZE, d);
	fclose(d);

	printf("os.fs ==>> Directory Table generated successfully!\n");
	
	uint8_t bitmap[512];
	uint8_t kernel_entry = 0b00000001;
	bitmap[0] = kernel_entry;

	FILE* bm = fopen("bitmap.bin", "wb");
	fwrite(bitmap, 1, 512, bm);
	fclose(bm);

	printf("os.fs ==>> Bitmap created successfully!\n");

	return 0;
}
