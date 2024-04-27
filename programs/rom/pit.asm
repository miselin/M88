bits 16
cpu 8086

segment _TEXT public align=16 use16 class=CODE

global configure_pit
global timer_handler
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

timer_handler:
    push ax
    push es

    mov ax, 0x40
    mov es, ax

    inc word [es:0x6C]
    jno .continue

    ; overflow, increment upper half
    inc word [es:0x6E]

    .continue:

    ; DX:AX = total tick count
    mov dx, [es:0x6E]
    mov ax, [es:0x6C]

    ; did a ms pass?
    mov cx, TICKS_PER_S
    div cx

    ; if AX < TICKS_PER_S, no ms passed
    cmp dx, 0
    jne .done

    mov al, 'T'
    out 0xE9, al

    .done:

    ; decrement shutoff counter if needed
    mov ax, [es:0x40]
    or ax, ax
    jz .no_shutoff

    dec ax
    mov [es:0x40], ax

    or ax, ax
    jnz .no_shutoff

    call fdc_motor_off              ; ticked down to zero, turn off FDC motor

    .no_shutoff:

    pop es
    pop ax
    ret

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
