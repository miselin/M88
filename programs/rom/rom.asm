bits 16
cpu 8086

segment _TEXT public align=16 use16 class=CODE

global start

extern call_option_roms
extern load_ivt
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
extern call_video_bios

..start:
start:
    jmp next
    next:

    cli

    ; are we an option ROM?
    ; skip some early init if so
    mov ax, 0xAA55
    mov di, 0x0000
    cmp [cs:di], ax
    je .option_rom

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

    ; interrupts are safe now
    sti

    ; POST #6 - interrupts are enabled
    mov al, 0x06
    out 0xE0, al
    out 0x80, al

    ; call VIDEO BIOS and other Option ROMs if present
    call call_option_roms

    ; POST #7 - ROMs have been called
    mov al, 0x07
    out 0xE0, al
    out 0x80, al

    ; set up keyboard controller
    ; call configure_kbc

    ; POST #7 - KBD is set up
    mov al, 0x07
    out 0xE0, al
    out 0x80, al

    jmp .not_option_rom

    .option_rom:

    mov si, mattbios
    call puts

    call load_ivt

    mov ax, 0xAA55
    mov di, 0x0000
    cmp [cs:di], ax
    jne .not_option_rom

    retf

    .not_option_rom:

    ; run the bootloader
    int 0x19

    cli
    hlt

segment _DATA public align=16 use16 class=DATA

mattbios db "Matt BIOS - Option ROM mode...", 13, 10, 0
