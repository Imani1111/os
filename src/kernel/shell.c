#include <shell.h>
#include <screen.h>
#include <ui.h>
#include <keyboard.h>
#include <task_mgr.h>

extern int cursor_x;
extern int cursor_y;

void read_command(){
	print_string("kernelshell> ", 0x0000ff00);
	while(1){
		char c = read_key();
		if (c != 0){
			draw_char(c, 0x0000ff00);
		}
	}
}

void shell_main()
{
	set_cursor_bounds(108, 492, 116, 392);
	set_cursor(108, 96);
	
	Window_t* win = open_window(100, 80, 400, 300, 0x00000000, 0x000000ff, "Shell");

	if (cursor_x != 108 || cursor_y != 96){
		print_string("SOMETHING IS STEALING MY CURSOR!", 0x00ff0000);
	}
	read_command();
}
