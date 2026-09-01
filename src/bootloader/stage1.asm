[bits 16]
[org 0x7c00]
	jmp 0x0000:main

main:
	xor ax, ax
	mov ds, ax
	mov es, ax
	mov ss, ax
	mov gs, ax
	mov sp, 0x7c00

	mov [boot_drive], dl
	mov si, message
	call print
	call readstage2	
	call jumptostage2

print:
	lodsb
	mov ah, 0x0e
	int 0x10
	cmp al, 0
	jne print
	ret

readstage2:
	mov ah, 0x02
	mov al, 2
	mov ch, 0
	mov cl, 2
	mov dh, 0
	mov dl, [boot_drive]
	mov bx, 0x8000
	
	int 0x13
	jc .disk_error
	jmp .success
.success:
	mov si, readsuccess
	call print
	ret
.disk_error:
	mov si, disk_error
	call print
	jmp $

jumptostage2:
	jmp 0x0000:0x8000

message: db "Hello from Real Mode!", 0x0d, 0x0a, 0x0
disk_error: db "Disk Error while reading bootloader stage 2!", 0x0d, 0x0a, 0x0
readsuccess: db "Stage 2 read successfully!", 0x0d, 0x0a, 0x0
boot_drive: db 0

times 510 - ($ - $$) db 0
dw 0xaa55


	
