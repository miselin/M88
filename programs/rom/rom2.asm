bits 16
cpu 8086

sig:
db 0x55
db 0xAA
sz:
db 0x01

start:
mov ax, 0xb800
mov ds, ax
mov al, 'Z'
mov ah, 0x07 ; gray on black
mov bx, 2
mov word [ds:bx], ax

retf

times 512 - ($-$sig) db 0
