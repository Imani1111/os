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
	
	uint32_t* n = my_malloc(10000);
	*n = 0x1;

	//TaskInit();
	//SpawnTask(InitDesktop, "Desktop");
	

	InitIDT();
	InitializePIT(100);
	send_byte_to_port(0x21, 0b11111100); // Unmask PIT(bit 0) and Keyboard(bit 1)
	
	while(1);
}

