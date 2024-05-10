bits 16
cpu 8086

segment _TEXT public align=16 use16 class=CODE

global load_ivt

extern int11
extern int12
extern int14
extern int15
extern int16
extern int17
extern int19
extern int1a

extern timerirq
extern kbirq

load_ivt:
  mov ax, 0
  mov ds, ax

  mov ax, cs

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

  cmp ax, 0xF000                    ; don't hook ROMBIOS interrupts if we're loaded as an option rom
  jne .not_biosrom

  mov word [ds:0x8 * 4], timerirq   ; IRQ0
  mov word [ds:0x8 * 4 + 2], ax

  mov word [ds:0x9 * 4], kbirq      ; IRQ1
  mov word [ds:0x9 * 4 + 2], ax

  mov word [ds:0x19 * 4], int19
  mov word [ds:0x19 * 4 + 2], ax

  .not_biosrom:

  mov word [ds:0x1a * 4], int1a
  mov word [ds:0x1a * 4 + 2], ax

  ret
