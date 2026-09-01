global LoadIDT

global isr33
extern HandleKeyboardInterrupt

global isr32
extern PITInterruptHandler
extern Schedule

LoadIDT:
	mov eax, [esp + 4]
	lidt[eax]
	ret

	
isr32:
	pusha
	call PITInterruptHandler
	
	push esp
	call Schedule
	add esp, 4
	mov esp, eax
	
	popa
	iret

isr33:
	pusha
	call HandleKeyboardInterrupt
	popa
	iret
