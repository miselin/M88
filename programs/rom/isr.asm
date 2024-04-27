bits 16
cpu 8086

segment _TEXT public align=16 use16 class=CODE

global isr
global _isr_array

extern _irqhandler
extern _isrhandler

; AL = interrupt number
isr:
    push bx
    push cx
    push dx
    push ds
    push es
    push di
    push si
    push bp

    push ax

    ; make sure we're in the BIOS data segment
    mov ax, 0xF000
    mov ds, ax
    mov es, ax

    call _isrhandler

    pop ax

    pop bp
    pop si
    pop di
    pop es
    pop ds
    pop dx
    pop cx
    pop bx
    pop ax

    iret

global _load_ivt

_load_ivt:
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
    add bx, 6  ; 8 bytes of instructions per ISR
    sub cx, 1
    jnz .loop

    ret

%macro ISR 1
  isr%1:
    push ax
    mov ax, %1
    jmp isr
%endmacro

%assign i 0
%rep 256
  ISR i
  %assign i i+1
%endrep

; segment rdata public align=4 use16 class=data

; _isr_array:
;   %assign i 0
;   %rep 256
;     dw isr %+ i
;     %assign i i+1
;   %endrep
