bits 16
cpu 8086

segment code public align=16 use16 class=code

global isr
extern irqhandler

; AL = interrupt number
isr:
    cmp al, 0x08
    jl .notirq

    cmp al, 0x0F
    jg .notirq

    jmp irqhandler

    .notirq:
    ret

global load_ivt
extern isr0

load_ivt:
    ; zero out the IVT area to start with
    mov cx, 512
    cld
    xor ax, ax
    mov es, ax
    mov di, ax
    rep stosw

    ; now load the ISR addresses

    xor di, di
    mov bx, isr0
    mov cx, 256
.loop:
    mov word [es:di], bx
    mov word [es:di + 2], 0xF000
    add di, 4
    add bx, 8  ; 8 bytes of instructions per ISR
    sub cx, 1
    jnz .loop

    ret
