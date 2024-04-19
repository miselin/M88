bits 16
cpu 8086

segment code public align=16 use16 class=code

global configure_pit
global timer_handler

configure_pit:
    mov al, 0b00110110 ; counter 0, lo/hi byte, rate generator, not BCD
    out 0x43, al

    ; set counter to 0xFFFF
    ; this is ~18.2 Hz (1.193182 MHz / 65536)
    mov al, 0xFF
    out 0x40, al
    out 0x40, al

    ; unmask IRQ0
    in al, 0x21
    and al, 0b11111110
    out 0x21, al

    ; set up our tick counter
    mov ax, 0x40
    mov es, ax
    mov word [es:0x15], 0

    mov word [es:0x00], 0

    ret

timer_handler:
    mov ax, 0x40
    mov es, ax

    mov ax, [es:0x15]
    inc ax

    ; 24 MHz crystal instead of 14.31818 MHz
    ; means the PIT runs 60% faster
    cmp ax, 30 ; 18
    jl .done

    mov al, 'T'
    out 0xE9, al

    mov ax, [es:0x00]
    inc ax
    cmp ax, 10
    jl .sevenseg

    xor ax, ax

    .sevenseg:
    out 0xF0, al

    mov [es:0x00], ax

    xor ax, ax

    .done:

    mov [es:0x15], ax

    ret