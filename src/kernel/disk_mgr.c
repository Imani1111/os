#include <disk_mgr.h>
#include <idt.h>
/*
inline void send_word_to_port(uint16_t port, const void* address, uint32_t count){
	asm volatile (
		"cld\n\t"
		"rep outsw"
		: "+S" (address), "+c" (count)
		: "d" (port)
		: "memory"
	);
}*/

int disk_read_sector(uint32_t lba, uint8_t* ram_buffer)
{
	while(read_byte_from_port(COMMAND_PORT) & (1 << 7));

	send_byte_to_port(DRIVE_REG, MASTER_DRIVE_MODE | ((lba >> 24) & 0x0F));
	send_byte_to_port(SECTOR_COUNT_REG, 1);
	send_byte_to_port(LBA_LOW, (lba & 0xFF));
	send_byte_to_port(LBA_MID, ((lba >> 8) & 0xFF));
	send_byte_to_port(LBA_HIGH, ((lba >> 16) & 0xFF));	

	send_byte_to_port(COMMAND_PORT, READ_FUNC);
	
	while(1){
		int status = read_byte_from_port(COMMAND_PORT);
		if (status & 0x01){
			//TODO: Write an error handler
			return -1;
		}else if(!(status & 0x80) && (status & (1 << 3))){
			break;
		}
	}

	uint16_t* target = (uint16_t*)ram_buffer;
	for (int i = 0; i < 256; i++){
		target[i] = read_word_from_port(DATA_PORT);
	}
	return 0;
}

int disk_write_sector(uint32_t lba, uint8_t* ram_buffer)
{
	while(read_byte_from_port(COMMAND_PORT) & (1 << 7));
	
	send_byte_to_port(DRIVE_REG, MASTER_DRIVE_MODE | ((lba >> 24) & 0xF));
	send_byte_to_port(SECTOR_COUNT_REG, 1);
	send_byte_to_port(LBA_LOW, (lba & 0xFF));
	send_byte_to_port(LBA_MID, ((lba >> 8) & 0xFF));
	send_byte_to_port(LBA_HIGH, ((lba >> 16) & 0xFF));

	send_byte_to_port(COMMAND_PORT, WRITE_FUNC);

	while(1){
		int status = read_byte_from_port(COMMAND_PORT);
		if (status & 0x01){
			return -1;
		}else if (!(status & 0x80) && (status & (1 << 3))){
			break;
		}
	}

	uint16_t* target = (uint16_t*)ram_buffer;
	for (int i = 0; i < 256; i++){
		send_word_to_port(DATA_PORT, target[i]);
	}

	send_byte_to_port(COMMAND_PORT, FLUSH_CACHE);
	return 0;
}
