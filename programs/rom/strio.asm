bits 16
cpu 8086

segment code public align=16 use16 class=code

global puts
global putnum

; ES:DI = pointer to null-terminated string
puts:
    push ax
    .loop:
    mov al, [es:di]
    inc di
    test al, al
    jz .done

    out 0xE9, al
    jmp .loop

    .done:
    pop ax
    ret
