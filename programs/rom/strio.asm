bits 16
cpu 8086

segment _TEXT public align=16 use16 class=CODE

global putc
global puts
global puthex
global puthex8
global putnum

; AL = character to print
putc:
    push ax
    push bx

    mov ah, 0x0E
    xor bh, bh
    mov bl, 0x07
    int 0x10

    pop bx
    pop ax
    ret

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

; AL = 4-bit nibble to display in hexadecimal
putnib:
    push ax
    push bx
    push cx
    and al, 0xF
    cmp al, 10
    jl .digit
    add al, 'A' - 10
    jmp .print
    .digit:
    add al, '0'
    .print:
    mov ah, 0x0E
    xor bh, bh    ; page 0
    mov bl, 0x07  ; gray on black
    mov cx, 0x01
    int 0x10
    pop cx
    pop bx
    pop ax
    ret

; AX = 16-bit number to display in hexadecimal
puthex:
    push ax
    push cx

    mov cx, 4

    xchg al, ah             ; swap bytes for correct rendering

    call putnib             ; print each nibble, unrolled loop
    shr ax, cl

    call putnib
    shr ax, cl

    call putnib
    shr ax, cl

    call putnib
    shr ax, cl

    pop cx
    pop ax
    ret

; AL = 8-bit number to display in hexadecimal
puthex8:
    push ax
    push cx

    mov cx, 4

    call putnib             ; print each nibble, unrolled loop
    shr ax, cl

    call putnib
    shr ax, cl

    pop cx
    pop ax
    ret

; AX = 16-bit number to display in decimal
putnum:
    push ax
    push bx
    push cx
    push dx
    push di
    push ds

    push bp
    mov bp, sp
    sub sp, 6           ; need 6 bytes of stack for the digit buffer

    mov di, sp          ; DI points to the start of the digit buffer

    mov cx, 10
    xor bx, bx
    xor dx, dx

    .loop:              ; main loop to extract digits
    test ax, ax
    jz .bufdone
    div cx              ; DX:AX / CX -> AX = quotient, DX = remainder
    add dl, '0'
    mov byte [ss:di+bx], dl
    inc bx
    xor dx, dx
    jmp .loop

    .bufdone:

    add di, bx          ; update DI to top of digit buffer
    mov cx, bx          ; move counter to CX for print loop

    .print:             ; print digits in reverse order
    dec di
    mov al, [ss:di]
    mov ah, 0x0E
    xor bh, bh          ; page 0
    mov bl, 0x07        ; gray on black
    int 0x10
    loop .print

    mov sp, bp
    pop bp

    pop ds
    pop di
    pop dx
    pop cx
    pop bx
    pop ax
    ret
