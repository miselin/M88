bits 16
cpu 8086

section .text

global configure_pic
global irqhandler
global unmask_irq

extern timer_handler
extern fdc_irq

configure_pic:
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
    mov al, 0b00000001 ; 8086 mode, normal EOI
    out 0x21, al
    out 0xA1, al

    ; mask all interrupts
    mov al, 0xFF
    out 0x21, al
    out 0xA1, al

    ret

; Unmasks an IRQ line
; AX = IRQ number
unmask_irq:
    push bx
    push cx
    mov cx, ax
    mov bx, 1
    shl bx, cl              ; bx = 1 << ax
    not bx                  ; flip bits to create mask
    in al, 0x21             ; read the current mask
    and al, bl              ; unmask the IRQ
    out 0x21, al            ; write the new mask
    pop cx
    pop bx
    ret
