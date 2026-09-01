#include <screen.h>
#include <idt.h>
#include <keyboard.h>
#include <pit.h>
#include <ui.h>
#include <task_mgr.h>
#include <mem_mgr.h>
#include <disk_mgr.h>
#include <string.h>
#include <shell.h>
#include <fs.h>
#include <mmap.h>

extern uint32_t first_allocatable_addr;
void kernel_main()
{
	asm volatile("cli");
	clear_screen(0x00000000);
	set_cursor_bounds(0, 640, 0, 480);
	Init_fs();
	init_pmm();
	init_vmm();
	
	uint32_t p = pmm_alloc_page();
	uint32_t* m = (uint32_t*)vmm_map_page(0xC0000000, p, READ_WRITE);
	uint32_t val = *m;
		
	//TaskInit();
	//SpawnTask(InitDesktop, "Desktop");
	
	//volatile uint32_t* unmapped_addr = (volatile uint32_t*)0x80000000;
	//uint32_t val = *unmapped_addr;	

	InitIDT();
	InitializePIT(100);
	send_byte_to_port(0x21, 0b11111100); // Unmask PIT(bit 0) and Keyboard(bit 1)
	
	while(1);
}

