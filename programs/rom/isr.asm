bits 16
cpu 8086

section .text

global load_ivt
global load_ivt_128

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
extern fdc_irq

extern puts

load_ivt:
  mov ax, 0
  mov ds, ax

  mov ax, cs

  cmp ax, 0xF000                    ; if we are ROMBIOS, install an unhandled interrupt handler for first 128 interrupts
  jne .after_default

  mov di, 0
  mov cx, 128

  .set_defaults:
  mov word [ds:di], 0xFF53          ; default handler (just an IRET)
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

  mov word [ds:0x9 * 4], fixedaddr_int9
  mov word [ds:0x9 * 4 + 2], ax

  mov word [ds:0x11 * 4], fixedaddr_int11
  mov word [ds:0x11 * 4 + 2], ax

  mov word [ds:0x12 * 4], fixedaddr_int12
  mov word [ds:0x12 * 4 + 2], ax

  mov word [ds:0x14 * 4], fixedaddr_int14
  mov word [ds:0x14 * 4 + 2], ax

  mov word [ds:0x15 * 4], fixedaddr_int15
  mov word [ds:0x15 * 4 + 2], ax

  mov word [ds:0x16 * 4], fixedaddr_int16
  mov word [ds:0x16 * 4 + 2], ax

  mov word [ds:0x17 * 4], fixedaddr_int17
  mov word [ds:0x17 * 4 + 2], ax

  mov word [ds:0x1a * 4], fixedaddr_int1a
  mov word [ds:0x1a * 4 + 2], ax

  cmp ax, 0xF000                                ; don't hook ROMBIOS / VGA interrupts if we're loaded as an option rom
  jne .not_biosrom

  mov word [ds:0x8 * 4], fixedaddr_int8         ; IRQ0
  mov word [ds:0x8 * 4 + 2], ax

  mov word [ds:0x10 * 4], fixedaddr_int10
  mov word [ds:0x10 * 4 + 2], ax

  mov word [ds:0x13 * 4], fixedaddr_int13
  mov word [ds:0x13 * 4 + 2], ax

  mov word [ds:0x40 * 4], fixedaddr_int13       ; INT 40h will point at ROMBIOS INT 13h handler (which is a dummy handler)
  mov word [ds:0x40 * 4 + 2], ax

  mov word [ds:0x19 * 4], fixedaddr_int19
  mov word [ds:0x19 * 4 + 2], ax

  mov word [ds:0x1C * 4], fixedaddr_emptyint    ; INT 1C defaults to an empty ISR (it's "System Timer Tick" and user-hookable)
  mov word [ds:0x1C * 4 + 2], 0xF000

  mov word [ds:0x44 * 4], 0xFA6E    ; VGA font

  .not_biosrom:

  ret

; load a default handler for remaining interrupts
; called after the POST stack is moved out of the way
load_ivt_128:
  mov ax, cs

  mov di, 128 * 4
  mov cx, 128

  .set_defaults:
  mov word [ds:di], fixedaddr_emptyint          ; default handler (just an IRET)
  add di, 2
  mov word [ds:di], ax
  add di, 2
  loop .set_defaults

; default handlers for IRQ0-7
irq0to7:
  push ax
  mov al, 0x20
  out 0x20, al
  pop ax
  iret

section .rdata

unhandledstr db "Unhandled Interrupt", 13, 10, 0

section .emptyint
fixedaddr_emptyint:
iret

section .int5
fixedaddr_int5:
iret

section .int8
fixedaddr_int8:
jmp timerirq

section .int9
fixedaddr_int9:
jmp kbirq

section .int0e
fixedaddr_int0e:
jmp fdc_irq

section .int10
fixedaddr_int10:
jmp int10

section .int11
fixedaddr_int11:
jmp int11

section .int12
fixedaddr_int12:
jmp int12

section .int13
fixedaddr_int13:
jmp int13

section .int14
fixedaddr_int14:
jmp int14

section .int15
fixedaddr_int15:
jmp int15

section .int16
fixedaddr_int16:
jmp int16

section .int17
fixedaddr_int17:
jmp int17

section .int19
fixedaddr_int19:
jmp int19

section .int1a
fixedaddr_int1a:
jmp int1a
