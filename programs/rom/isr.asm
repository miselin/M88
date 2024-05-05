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
    push sp

    ; make sure we're in the BIOS data segment
    mov ax, 0xF000
    mov ds, ax
    mov es, ax

    call _isrhandler

    add sp, 4 ; remove the pushed AX, SP

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

%macro ISR 1
  global isr%1
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

segment rdata public align=4 use16 class=data

_isr_array:
  %assign i 0
  %rep 256
    dw isr %+ i
    %assign i i+1
  %endrep
