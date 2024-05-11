bits 16
cpu 8086

segment _TEXT public align=16 use16 class=CODE

global count_memory

extern puts
extern putnum

count_memory:
    mov si, memcount
    call puts

    mov ax, 0x1000              ; count in 1K blocks, starting at 64K
    mov bx, 64
    mov cx, 4

    .loop:
    mov ds, ax
    mov word [ds:0], 0xAAAA
    cmp word [ds:0], 0xAAAA
    jne .done

    add ax, 0x40
    inc bx

    push ax
    mov ax, bx
    call putnum
    mov si, kb
    call puts
    pop ax
    jmp .loop

    .done:
    mov ax, 0x40
    mov ds, ax
    sub bx, 4                   ; steal 4K at top of memory for BIOS use
    mov [ds:0x13], bx           ; store memory size in KB

    mov si, crnl
    call puts

    ret

segment _DATA public align=16 use16 class=DATA

memcount db "Memory check...", 13, 10, 0
kb db "K", 13, 0
crnl db 13, 10, 0

