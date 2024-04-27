bits 16
cpu 8086

segment _TEXT public align=16 use16 class=CODE

global start

extern _rommain
extern _call_option_roms
extern _load_ivt
extern configure_pic
extern configure_pic_bochs
extern configure_pit
extern configure_kbc
extern configure_dma
extern configure_fdc
extern fdc_read_drive0
extern delay_ticks
extern puts
extern putnum

..start:
start:

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

; POST #1 - we're alive, about to check memory
mov al, 0x01
out 0xF0, al

; do any DMA controller config before touching memory
call configure_dma

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

; before we jump to C, make sure CS=DS=ES
mov ax, 0xF000
mov ds, ax
mov es, ax

; it's now safe to go to C code as we have memory up and running
call _rommain

cli
hlt

; set up the IVT with our interrupt handlers
call _load_ivt

; POST #3 - IVT is set up
mov al, 0x03
out 0xF0, al

; set up interrupt controller
; call configure_pic
call configure_pic_bochs

; POST #4 - PIC is set up
mov al, 0x04
out 0xF0, al

; set up timer controller
call configure_pit

; POST #5 - PIT is set up
mov al, 0x05
out 0xF0, al

; set up keyboard controller
call configure_kbc

; POST #6 - KBD is set up
mov al, 0x06
out 0xF0, al

; interrupts are safe now
sti

; POST #7 - about to call Option ROMs
mov al, 0x07
out 0xF0, al

call _call_option_roms

mov ax, 0xb800
mov ds, ax
mov al, 'A'
mov ah, 0x07 ; gray on black
mov bx, 0
mov word [ds:bx], ax

; configure FDC
call configure_fdc

; POST #8 - system is ready, attempting to boot application code
mov al, 0x08
out 0xF0, al

; attempt to load the boot sector from floppy
mov ax, 0x0000
mov es, ax
mov dx, 0x7C00
mov cx, 0x200           ; 512 bytes
mov bx, 0x0000          ; cylinder
call fdc_read_drive0

cmp ax, 0
je .boot_ok

; POST #9 - boot failed, can't read from FDC
mov al, 0x09
out 0xF0, al

.boot_ok:
jmp forever
jmp 0x0000:0x7C00

forever:
sti
hlt
jmp forever
