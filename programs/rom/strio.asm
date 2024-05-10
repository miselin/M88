bits 16
cpu 8086

segment _TEXT public align=16 use16 class=CODE

global puts

; CS:SI = pointer to null-terminated string in ROM
puts:
    push ax
    push bx
    push cx

    mov ah, 0x0E
    xor bh, bh    ; page 0
    mov bl, 0x07  ; gray on black
    mov cx, 0x01

    .loop:
    mov al, [cs:si]
    test al, al
    jz .done

    int 0x10

    inc si
    jmp .loop

    .done:
    pop cx
    pop bx
    pop ax
    ret
