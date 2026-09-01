[bits 32]

section .text.prologue

global launch_kernel
extern kernel_main

launch_kernel:
	call kernel_main
.hang:
	cli
	jmp $
