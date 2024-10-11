bits 16
cpu 8086

extern call_option_roms
extern load_ivt
extern load_ivt_128
extern configure_pic
extern configure_pic_bochs
extern configure_pit
extern configure_kbc
extern configure_dma
extern configure_fdc
extern fdc_read_drive0
extern delay_ticks
extern puts
extern puts_ext
extern putnum
extern puthex
extern call_video_bios
extern count_memory
extern beep
extern configure_serial
extern configure_parallel
extern test_dma
extern logo
extern end_logo
extern configure_i2c

section .entry

global start

; basic POST test before jumping to main BIOS code
start:
    cli

    ; POST #0 - we're alive
    mov al, 0x00
    out 0xE0, al
    out 0x80, al

    ; CPU test
    mov ax, 0b1010101010101010
    mov ds, ax
    mov bx, ds
    mov es, bx
    mov cx, es
    mov ss, cx
    mov dx, ss
    mov bp, dx
    mov sp, bp
    mov si, bp
    mov di, si
    cmp di, 0b1010101010101010
    je .cpu_ok

    hlt

    .cpu_ok:

    ; POST #1 - CPU is OK
    mov al, 0x01
    out 0xE0, al
    out 0x80, al

    jmp entry

section .text
entry:
    ; default data segment at start of memory
    xor ax, ax
    mov ds, ax

    ; set up BIOS stack at 30:FF (256 bytes) to bootstrap until we know how much memory is present
    mov ax, 0x30
    mov ss, ax
    mov sp, 0xFF
    mov bp, sp

    ; does the first 64K of memory work?
    mov ax, 0x55AA
    mov di, 0xFFFE
    mov [ds:di], ax
    cmp [ds:di], ax
    je .memory_ok

    hlt

    .memory_ok:

    ; zero the BDA
    xor ax, ax
    mov es, ax
    mov di, 0x400
    mov cx, 0x80
    rep stosw

    ; set initial BDA variables
    mov word [0x410], 0b0000000100110000    ; equipment flags
                                            ; 1 DMA, 80x25 16 color, no FPU, no other flags
    mov word [0x413], 64                    ; 64K of memory currently, we'll count more later

    ; POST #2 - first 64K of memory is OK
    mov al, 0x02
    out 0xE0, al
    out 0x80, al

    ; do DMA controller config now that memory works
    call configure_dma

    ; set up the IVT with our interrupt handlers
    call load_ivt

    ; ; POST #3 - IVT is set up
    mov al, 0x03
    out 0xE0, al
    out 0x80, al

    ; ; set up interrupt controller
    call configure_pic

    ; ; POST #4 - PIC is set up
    mov al, 0x04
    out 0xE0, al
    out 0x80, al

    ; set up timer controller
    call configure_pit

    ; POST #5 - PIT is set up
    mov al, 0x05
    out 0xE0, al
    out 0x80, al

    ; emit POST beep now that PIT is ready
    call beep

    ; interrupts are safe now
    sti

    ; POST #6 - interrupts are enabled
    mov al, 0x06
    out 0xE0, al
    out 0x80, al

    ; call VIDEO BIOS
    call call_video_bios

    ; POST #7 - VIDEO BIOS has been called
    mov al, 0x07
    out 0xE0, al
    out 0x80, al

    ; count memory before running Option ROMs (as they may need to know)
    call count_memory

    ; POST #8 - memory count complete
    mov al, 0x08
    out 0xE0, al
    out 0x80, al

    ; swap stack to top of memory now that we know where that is
    mov ax, 0x40
    mov ds, ax
    mov ax, [ds:0x13]               ; memory size in KB, minus 4K
    add ax, 1                       ; skip 1K area for EBDA
    mov cx, 6                       ; left shift 6 places to turn into segment (1 KB >> 4)
    shl ax, cl
    mov ss, ax
    mov sp, 0xBFF                   ; 3K stack

    call load_ivt_128

    ; POST #9 - stack moved
    mov al, 0x09
    out 0xE0, al
    out 0x80, al

    ; set EBDA at top of memory
    mov ax, [ds:0x13]               ; memory size in KB, minus 4K
    shl ax, cl                      ; create segment from it
    mov ds, ax
    mov es, ax
    mov word [ds:0], 1              ; 1KB allocated to EBDA
    xor di, di                      ; zero out the rest of the EBDA
    xor ax, ax
    mov cx, 0x200
    rep stosw
    mov ax, 0x40                    ; store EBDA segment in BDA
    mov ds, ax
    mov [ds:0x0E], es

    call test_dma

    ; POST #10 - EBDA in place
    mov al, 0x10
    out 0xE0, al
    out 0x80, al

    ; set up keyboard controller
    call configure_kbc

    ; POST #11 - KBC configured
    mov al, 0x11
    out 0xE0, al
    out 0x80, al

    call configure_serial
    call configure_parallel

    ; POST #12 - serial/parallel ports configured
    mov al, 0x12
    out 0xE0, al
    out 0x80, al

    call call_option_roms

    ; POST #13 - Option ROMs called, configuring I2C if present
    mov al, 0x13
    out 0xE0, al
    out 0x80, al

    call configure_i2c

    ; POST #88 - about to call INT19
    mov al, 0x88
    out 0xE0, al
    out 0x80, al

    ; make sure interrupts are on again in case an option ROM turned them off
    sti

    call write_logo

    mov si, debug
    call puts

    mov dx, 0x2A3
    .forever:
    in al, dx
    jmp .forever

    ; run the bootloader
    mov dl, 0x80                ; prefer HDD
    int 0x19

    cli
    hlt

option_rom:
    mov ax, 0xAA55              ; verify we're running as an Option ROM
    mov di, 0x0000
    cmp [cs:di], ax
    jne start

    mov ax, 0x03                ; set video mode to 3, which will clear screen
    int 0x10

    ;mov si, mattbios
    ;call puts

    call write_logo

    call load_ivt               ; replace relevant IVT entries for this ROM

    retf

write_logo:
    push ax
    push dx
    push si
    push es
    push bp

    mov ax, cs
    mov es, ax

    xor dh, dh                  ; row 0

    mov si, logo
    .outer:
    cmp si, end_logo
    jge .done

    xor dl, dl                  ; column 0

    mov cx, 80
    xor bh, bh
    mov bl, 0x1F
    mov bp, si
    call puts_ext

    add si, 81
    inc dh
    jmp .outer
    .done:

    pop bp
    pop es
    pop si
    pop dx
    pop ax
    ret

section .rdata

mattbios db "Matt BIOS - Option ROM mode...", 13, 10, 0
debug db "Spinning reading port 2A3 forever...", 13, 10, 0

section .poweron
jmp 0xF000:start

section .optionrom_short
; short jump past description strings
; only used when running as an Option ROM
jmp short optionrom_short

section .optionrom_long
; long jump to actual entry point
optionrom_short:
jmp option_rom
