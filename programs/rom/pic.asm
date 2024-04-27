bits 16
cpu 8086

segment code public align=16 use16 class=code

global configure_pic
global configure_pic_bochs
global irqhandler

extern timer_handler
extern fdc_irq

configure_pic:
    ; ICW1
    xor ax, ax
    mov al, 0b00010111 ; ICW4 needed, SINGLE mode, 4-byte interval, edge-triggered
    out 0x20, al
    ; ICW2
    mov al, 8 ; remap IRQ0 to interrupt 8
    out 0x21, al
    ; ICW3 is skipped due to SINGLE mode
    ; ICW4
    mov al, 0b10000001 ; 8086 mode, normal EOI
    out 0x21, al

    ; mask all interrupts
    mov al, 0xFF
    out 0x21, al

    ret

; Bochs requires multiple PICs
configure_pic_bochs:
    ; ICW1
    xor ax, ax
    mov al, 0b00010101 ; ICW4 needed, cascade mode, 4-byte interval, edge-triggered
    out 0x20, al
    out 0xA0, al
    ; ICW2
    mov al, 8 ; remap IRQ0 to interrupt 8
    out 0x21, al
    mov al, 0x70 ; remap IRQ0 to interrupt 0x70
    out 0xA1, al
    ; ICW3
    mov al, 4 ; IRQ2 is connected to the slave
    out 0x21, al
    mov al, 2 ; IRQ2 is connected to the master
    out 0xA1, al
    ; ICW4
    mov al, 0b10000001 ; 8086 mode, normal EOI
    out 0x21, al
    out 0xA1, al

    ; mask all interrupts
    mov al, 0xFF
    out 0x21, al
    out 0xA1, al

    ret

irqhandler:
    cmp al, 0x08
    jne .not_pit
    call timer_handler
    .not_pit:

    cmp al, 0x0E
    jne .not_fdc
    call fdc_irq
    .not_fdc:

    ; EOI
    mov al, 0x20
    out 0x20, al

    ret
