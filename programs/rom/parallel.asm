bits 16
cpu 8086

section .text

global configure_parallel

configure_parallel:
    mov ax, 0x40
    mov ds, ax

    .port1:
    mov dx, 0x379
    in al, dx
    cmp al, 0xFF
    je .port2

    dec dx
    mov [ds:0x08], dx

    .port2:
    mov dx, 0x279
    in al, dx
    cmp al, 0xFF
    je .port3

    dec dx
    mov [ds:0x0A], dx

    .port3:
    mov dx, 0x3BD
    in al, dx
    cmp al, 0xFF
    je .done

    dec dx
    mov [ds:0x0C], dx

    .done:
    ret
