bits 16
cpu 8086

segment _TEXT public align=16 use16 class=CODE

global load_ivt

extern int10
extern int11
extern int12
extern int13
extern int14
extern int15
extern int16
extern int17
extern int19
extern int1a

extern timerirq
extern kbirq

extern puts

load_ivt:
  mov ax, 0
  mov ds, ax

  mov ax, cs

  cmp ax, 0xF000                    ; if we are ROMBIOS, install an unhandled interrupt handler for first 128 interrupts
  jne .after_default

  mov di, 0
  mov si, _isr_array
  mov cx, 128

  .set_defaults:
  mov bx, word [cs:si]
  add si, 2
  mov word [ds:di], bx
  add di, 2
  mov word [ds:di], ax
  add di, 2
  loop .set_defaults

  mov di, 0x08 * 4
  mov cx, 8
  .set_irq0to7:
  mov word [ds:di], irq0to7
  add di, 4
  loop .set_irq0to7

  .after_default:

  mov word [ds:0x11 * 4], int11
  mov word [ds:0x11 * 4 + 2], ax

  mov word [ds:0x12 * 4], int12
  mov word [ds:0x12 * 4 + 2], ax

  mov word [ds:0x14 * 4], int14
  mov word [ds:0x14 * 4 + 2], ax

  mov word [ds:0x15 * 4], int15
  mov word [ds:0x15 * 4 + 2], ax

  mov word [ds:0x16 * 4], int16
  mov word [ds:0x16 * 4 + 2], ax

  mov word [ds:0x17 * 4], int17
  mov word [ds:0x17 * 4 + 2], ax

  cmp ax, 0xF000                    ; don't hook ROMBIOS / VGA interrupts if we're loaded as an option rom
  jne .not_biosrom

  mov word [ds:0x8 * 4], timerirq   ; IRQ0
  mov word [ds:0x8 * 4 + 2], ax

  mov word [ds:0x10 * 4], int10
  mov word [ds:0x10 * 4 + 2], ax

  mov word [ds:0x13 * 4], int13
  mov word [ds:0x13 * 4 + 2], ax

  mov word [ds:0x19 * 4], int19
  mov word [ds:0x19 * 4 + 2], ax

  .not_biosrom:

  mov word [ds:0x9 * 4], kbirq      ; IRQ1
  mov word [ds:0x9 * 4 + 2], ax

  mov word [ds:0x1a * 4], int1a
  mov word [ds:0x1a * 4 + 2], ax

  mov word [ds:0x44 * 4], 0xFA6E    ; VGA font

  ret

; default handlers for IRQ0-7
irq0to7:
  push ax
  mov al, 0x20
  out 0x20, al
  pop ax
  iret

unhandled:
  ; unhandled ISR, oh no!
  add al, 0x40
  out 0xE0, al
  out 0x80, al

  mov si, unhandledstr
  call puts

  cli
  hlt

  iret

%macro ISR 1
  isr%1:
    push ax
    mov ax, %1
    jmp unhandled
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

segment _DATA public align=16 use16 class=DATA

unhandledstr db "Unhandled Interrupt", 13, 10, 0
