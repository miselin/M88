bits 16
cpu 8086

segment _TEXT public align=16 use16 class=CODE

global configure_pit
global timerirq
global delay_ticks
global delay_s
global set_fdc_shutoff_counter
extern fdc_motor_off

; Bochs/standard PC with 14.31818 MHz crystal
TICKS_PER_S equ 18

; Custom PC with 24 MHz crystal
; TICKS_PER_S equ 30

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

    ; set up the daily tick counter
    mov ax, 0x40
    mov es, ax
    mov word [es:0x6C], 0
    mov word [es:0x6E], 0
    mov word [es:0x70], 0  ; 24-hour rollover flag
    mov word [es:0x40], 0  ; motor shutoff counter

    ret

timerirq:
    push ax
    push ds

    mov ax, 0x40
    mov ds, ax

    inc word [ds:0x6C]              ; increment daily tick counter
    jno .continue
    inc word [ds:0x6E]              ; overflowed, increment upper half
    .continue:

    cmp byte [ds:0x40], 0           ; decrement shutoff counter if it is non-zero
    jz .no_shutoff
    dec byte [ds:0x40]
    jnz .no_shutoff                 ; if the decrement hit zero, we'll turn off FDC motor
    call fdc_motor_off
    .no_shutoff:

    mov al, 0x20                    ; EOI
    out 0x20, al

    pop ds
    pop ax
    iret

; Read the current counter value from the PIT (channel 0)
; Returns the counter value in AX
read_pit_counter:
    cli
    mov al, 0x00
    out 0x43, al            ; latch current counter value

    in al, 0x40
    mov ah, al
    in al, 0x40
    xchg al, ah
    sti
    ret

; Delay for the specified number of ticks passed in AX
delay_ticks:
    push es
    push ax
    push bx

    mov bx, ax
    call read_pit_counter       ; get current counter value
    sub ax, bx                  ; calculate target value (overflow is OK)
    xchg ax, bx                 ; BX = target value

    .loop:
    sti
    hlt                         ; wait for IRQ before checking for updated value
    call read_pit_counter       ; get counter
    cmp ax, bx
    jbe .loop

    pop bx
    pop ax
    pop es
    ret

; Delay for the specified number of seconds passed in AX
delay_s:
    push cx
    mov cx, TICKS_PER_S
    mul cx
    call delay_ticks

    pop cx
    ret

; Set the FDC motor shutoff counter to the value in AX (in ticks)
set_fdc_shutoff_counter:
    push es
    push ax
    mov ax, 0x40
    mov es, ax
    pop ax
    mov [es:0x40], ax
    pop es
    ret
