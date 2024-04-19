bits 16
cpu 8086

segment code public align=16 use16 class=code

..start:

extern _rommain
extern call_option_roms
extern load_ivt
extern configure_pic
extern configure_pic_bochs
extern configure_pit

cli

mov al, 0x00
out 0xF0, al

; default data segment at start of memory
mov ax, 0x50
mov ds, ax

; set up BIOS stack at 30:FF (256 bytes) to bootstrap until we know how much memory is present
mov ax, 0x30
mov ss, ax
mov sp, 0xFF
mov bp, sp

; POST #1 - we're alive, poke the 7-seg
mov al, 0x01
out 0xF0, al

; does memory work?
mov ax, 0x55AA
mov di, 0x0000
mov [ds:di], ax
cmp [ds:di], ax
je .memory_ok

hlt

.memory_ok:

; cheat: we know we have 64K (due to a demux schematic bug, instead of 128K)
; put the stack right at the top of that (0x0FFFF)
mov ax, 0x0000
mov ss, ax
mov sp, 0xFFFF
mov bp, sp

; POST #2 - memory is OK
mov dx, 2
push dx
pop ax

out 0xF0, al

; set up the IVT with our interrupt handlers
call load_ivt

; POST #3 - IVT is set up
mov al, 0x03
out 0xF0, al

; set up interrupt controller
call configure_pic
; call configure_pic_bochs

; POST #4 - PIC is set up
mov al, 0x04
out 0xF0, al

; set up timer controller
call configure_pit

; POST #5 - PIT is set up
mov al, 0x05
out 0xF0, al

; interrupts are safe now
sti

; POST #6 - system is ready
mov al, 0x06
out 0xF0, al

; call call_option_roms

; mov ax, 0xb800
; mov ds, ax
; mov al, 'A'
; mov ah, 0x07 ; gray on black
; mov bx, 0
; mov word [ds:bx], ax

; 7-seg display
; mov al, 0x01
; out 0xF0, al

mov al, 'A'
out 0xE9, al

; call _rommain

forever:
sti
hlt
jmp forever

segment data public align=4 use16 class=data

segment rdata public align=4 use16 class=data

group dgroup data rdata