#include <fs.h>
#include <disk_mgr.h>
#include <screen.h>
#include <string.h>

static uint8_t directory_buffer[DIRECTORY_TABLE_SIZE];
uint8_t bm_sector[512];
static uint8_t bitmap_cache[8];
FileEntry_t* root_directory = (FileEntry_t*)directory_buffer;

void Init_fs()
{
	uint8_t buffer[512];
	disk_read_sector(SUPERBLOCK_SECTOR, buffer);

	SuperBlock* superblock = (SuperBlock*)buffer;
	if (superblock->magic != FS_MAGICNUMBER){
		print_string("File system not found!", 0x00FF0000);
		return;
	}

	for (int i = 0; i < 64; i++){
		disk_read_sector(11 + i, (uint8_t*)(root_directory + (i * 512)));
	}
	print_string("File System Mounted Successfully!\n", 0x0000FF00);
	
	disk_read_sector(BITMAP_LBA, bm_sector);
	kmemcpy((void*)bitmap_cache, bm_sector, sizeof(bitmap_cache));
	if (bitmap_cache[0] & 0b00000001){
		print_string("Bitmap successfully cached\n", 0x0000ff00);
	}
}

FileEntry_t* find_file(const char* file_name)
{
	for (int i = 0; i < 512; i++){
		if (root_directory[i].name[0] == '\0'){
			continue;
		}
		if (kstrcmp((uint8_t*)root_directory[i].name, (uint8_t*)file_name) == 0){
			return &root_directory[i];
		}
	}
	return NULL;
}

int is_block_free(uint8_t* bitmap, uint32_t lba){
	int byte_idx = lba / 8;
	int bit_idx = lba % 8;
	if (bitmap[byte_idx] & (1 << bit_idx)){
		return 1;
	}
	return 0;
}

void mark_block_free(uint8_t* bitmap,  uint32_t lba){
	int byte_idx = lba / 8;
	int bit_idx = lba % 8;
	bitmap[byte_idx] &= ~(1 << bit_idx);
}

void mark_block_used(uint8_t* bitmap, uint32_t lba){
	int byte_idx = lba / 8;
	int bit_idx = lba % 8;
	bitmap[byte_idx] |= (1 << bit_idx);
}

uint32_t find_free_sector(uint8_t* bitmap, uint32_t total_sectors){
	for (uint32_t byte = 0; byte < total_sectors / 8; byte++){
		if (bitmap[byte] == 0xFF) continue;

		for (uint32_t bit = 0; bit < 8; bit++){
			if (!(bitmap[byte] & (1 << bit))){
				uint32_t lba = (byte * 8) + bit;
				mark_block_used(bitmap, lba);
				return lba;	
			}
		}
	}
	return 0;
}
