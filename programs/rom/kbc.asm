bits 16
cpu 8086

segment _TEXT public align=16 use16 class=CODE

extern unmask_irq
extern puts
extern delay_ticks

global configure_kbc
global kbirq

; PS2_CMD = 0x64  (system A2 -> KBC A0 pin)
; PS2_DATA = 0x60

kbc_wait_write:
    in al, 0x64
    test al, 2
    jnz kbc_wait_write
    ret

kbc_wait_read:
    in al, 0x64
    test al, 1
    jz kbc_wait_read
    ret

; write a ps/2 controller command
kbc_write_cmd:
    push ax
    call kbc_wait_write
    pop ax

    out 0x64, al
    ret

; read a byte from the ps/2 controller
kbc_read_cmd:
    call kbc_wait_read

    in al, 0x64
    ret

; write the byte in al to the first device
kbc_write_dev0:
    push ax
    call kbc_wait_write
    pop ax

    out 0x60, al
    ret

; read a byte from the first device, return in al
kbc_read_dev0:
    call kbc_wait_read

    in al, 0x60
    ret

; read a byte from the first device, without blocking
; sets CF if no byte is available
kbc_read_dev0_nb:
    in al, 0x64
    test al, 1
    jz .no_byte

    in al, 0x60
    clc
    ret

    .no_byte:
    stc
    ret

configure_kbc:
    ; disable all devices
    mov al, 0xAD
    call kbc_write_cmd

    mov al, 0xA7
    call kbc_write_cmd

    ; disable IRQs
    mov al, 0x20
    call kbc_write_cmd
    call kbc_read_dev0
    and al, ~3
    or al, 0x40

    mov bx, ax
    mov al, 0x60
    call kbc_write_cmd
    mov al, bl
    call kbc_write_dev0

    ; self-test the PS/2 controller
    mov al, 0xAA
    call kbc_write_cmd
    call kbc_read_dev0
    cmp al, 0x55
    jne .fail

    ; enable first port
    mov al, 0xAE
    call kbc_write_cmd
    mov al, 0xA8
    call kbc_write_cmd

    ; reset devices
    .resend:
    mov al, 0xFF
    call kbc_write_dev0
    call kbc_read_dev0 ; ack, TODO: check for 0xFE (resend)
    cmp al, 0xFE
    je .resend

    cmp al, 0xFA
    jne .fail
    call kbc_read_dev0 ; self-test result
    cmp al, 0xAA
    jne .fail

    ; turn on num lock
    mov al, 0xED
    call kbc_write_dev0
    mov al, 0x02
    call kbc_write_dev0

    ; consume the ACK to ensure the buffer is clear
    call kbc_read_dev0

    mov ax, 2                       ; hold for a bit to let things settle before
    call delay_ticks                ; we clear the buffer and enable IRQs

    .clearbuf:                      ; clear the KB buffer to avoid stray IRQs
    clc                             ; sometimes we get multiple ACKs and those would
    call kbc_read_dev0_nb           ; fire an unwanted IRQ when we enable it below
    jnc .clearbuf

    mov ax, 1                       ; unmask IRQ1
    call unmask_irq

    mov al, 0x20                    ; enable KB IRQs
    call kbc_write_cmd
    call kbc_read_dev0
    or al, 3
    mov bx, ax
    mov al, 0x60
    call kbc_write_cmd
    mov al, bl
    call kbc_write_dev0

    ret

    .fail:
    mov si, kbfail
    call puts

    cli
    hlt

kbirq:
    push ax
    push si
    mov si, kbstr
    call puts

    call kbc_read_dev0

    ; TODO...

    mov al, 0x20                    ; EOI
    out 0x20, al

    pop si
    pop ax
    iret

segment _DATA public align=16 use16 class=DATA

kbstr db "Keyboard IRQ", 13, 10, 0

kbfail db "Keyboard initialization failed", 13, 10, 0
