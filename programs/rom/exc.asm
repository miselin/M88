bits 16
cpu 8086

extern puts
extern puthex8

section .text

global int0
global int1
global int2
global int3
global int4

int0:
    mov al, 0
    jmp common_exc

int1:
    mov al, 1
    jmp common_exc

int2:
    mov al, 2
    jmp common_exc

int3:
    mov al, 3
    jmp common_exc

int4:
    mov al, 4
    jmp common_exc


; AL = which
common_exc:
    mov si, cpuexcept
    call puts
    call puthex8

    mov si, exceptions
    xor ah, ah
    shl al, 1           ; AL * 2
    add si, ax          ; exceptions[N]
    call puts

    cli
    .l:
    hlt
    jmp .l

section .rdata

cpuexcept db "CPU Exception #", 0

divide0 db "Divide by zero", 0
singlestep db "Single step", 0
nmi db "Non-maskable Interrupt", 0
breakpoint db "Breakpoint", 0
overflow db "Overflow", 0

exceptions:
    dw divide0
    dw singlestep
    dw nmi
    dw breakpoint
    dw overflow
