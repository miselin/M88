bits 16
cpu 8086

segment code public align=16 use16 class=code

global configure_kbc

; PS2_CMD = 0x64  (system A2 -> KBC A0 pin)
; PS2_DATA = 0x60

configure_kbc:
    ; disable & flush to start with
    mov al, 0xAD
    out 0x64, al

    in al, 0x60

    ; read config
    mov al, 0x20
    out 0x64, al

    in al, 0x60
    or al, 0x4
    and al, 0b10111100
    mov bl, al

    ; write updated config
    mov al, 0x60
    out 0x64, al
    mov al, bl
    out 0x60, al

    mov al, 0xAA
    out 0x64, al

    in al, 0x60
    cmp al, 0x55
    jne .fail

    ; enable first port
    mov al, 0xAE
    out 0x64, al

    ; reset first port
    mov al, 0xFF
    out 0x60, al

    in al, 0x60

    ret

    .fail:
    cli
    hlt
