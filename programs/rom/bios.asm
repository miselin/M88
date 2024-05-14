bits 16
cpu 8086

segment _TEXT public align=16 use16 class=CODE

global int10
global int11
global int12
global int13
global int14
global int15
global int16
global int17
global int19
global int1a

extern puts
extern puthex
extern puthex8
extern delay_ticks
extern read_pit_counter
extern kbc_sync_flags_leds

F_CF        equ 1 << 0
F_PF        equ 1 << 2
F_AF        equ 1 << 4
F_ZF        equ 1 << 6
F_SF        equ 1 << 7
F_TF        equ 1 << 8
F_IF        equ 1 << 9
F_DF        equ 1 << 10
F_OF        equ 1 << 11
F_IOPL      equ 3 << 12
F_NT        equ 1 << 14
F_RF        equ 1 << 15

int10:
    ; Some Option ROMs call INT 10h
    ; Make sure that works.
    iret

int11:
    ; INT 11 - BIOS Equipment Determination
    push ds
    mov ax, 0x40
    mov ds, ax
    mov ax, [ds:0x10]   ; BIOS Equipment List flags
    pop ds
    iret

int12:
    ; INT 12 - Get Memory Size
    push ds
    mov ax, 0x40
    mov ds, ax
    mov ax, [ds:0x13]   ; Memory size in KB
    pop ds
    iret

int13:
    ; Dummy INT 13h just in case

    push si
    mov si, unimpl13
    call puts
    pop si

    ; set carry flag, unimplemented function
    mov ah, 0x86
    push bp
    mov bp, sp
    or byte [bp + 6], 1  ; skip bp, ip, cs in stack to get to flags
    pop bp

    iret

int14:
    ; INT 14 - Serial Port Services

    cmp ah, 0x00
    je int14_00

    cmp ah, 0x01
    je int14_01

    cmp ah, 0x02
    je int14_02

    cmp ah, 0x03
    je int14_03

    push si
    mov si, unimpl14
    call puts
    pop si

    iret

int14_00:
    push cx
    push si

    ; AH = 0x00 - Initialize Serial Port
    ; AL = params
    ; DX = port
    shl dx, 1
    mov si, dx
    add si, serialports
    mov dx, [cs:si]

    ; parity, data size, stop bits
    mov cx, ax
    and cx, 0x1F

    ; baud rate divisor
    and ax, 0xFF
    mov cx, 4 ; two bytes per divisor
    shr ax, cl
    mov si, serialdivisors
    add si, ax
    mov bx, [cs:si]

    xor ax, ax
    add dx, 1
    out dx, al          ; disable interrupts to start with

    mov al, 0x80        ; set baud rate divisor
    add dx, 2
    out dx, al

    mov al, bl          ; low byte of divisor
    sub dx, 3
    out dx, al

    mov al, bh          ; high byte of divisor
    add dx, 1
    out dx, al

    mov al, cl          ; set data size, parity, stop bits
    add dx, 2
    out dx, al

    mov al, 0xC7        ; enable FIFO, clear it, 14-byte threshold
    sub dx, 1
    out dx, al

    mov al, 0x0B        ; enable IRQs, RTS/DSR
    add dx, 2
    out dx, al

    mov al, 0x0F        ; set in normal mode
    out dx, al

    pop si
    pop cx
    iret

int14_01:
    push si

    ; AH = 0x01 - Send One Character
    ; AL = characer to send
    ; DX = port
    shl dx, 1
    mov si, dx
    add si, serialports
    mov dx, [cs:si]

    push ax

    ; wait for transmitter buffer to be empty
    add dx, 5
    .loop:
    in al, dx
    and al, 0x20
    jz .loop

    pop ax

    ; send the character
    sub dx, 5
    out dx, al

    ; no error
    xor ah, ah

    pop si
    iret

int14_02:
    push si

    ; AH = 0x02 - Receive One Character
    ; DX = port
    shl dx, 1
    mov si, dx
    add si, serialports
    mov dx, [cs:si]

    push ax

    ; wait for data to be ready
    add dx, 5
    .loop:
    in al, dx
    and al, 0x1
    jz .loop

    pop ax

    ; receive the character
    in al, dx

    ; no error
    xor ah, ah

    pop si
    iret

int14_03:
    push si

    ; AH = 0x03 - Get Port Status
    ; DX = port
    shl dx, 1
    mov si, dx
    add si, serialports
    mov dx, [cs:si]

    ; read Line Status
    add dx, 5
    in al, dx
    mov ah, al

    ; read Modem Status
    add dx, 1
    in al, dx

    ; Line Status is top byte, Modem Status is bottom byte
    xchg ah, al

    pop si
    iret

int15:
    ; INT 15 - System Services

    cmp ah, 0x4F
    je int15_4f

    cmp ah, 0x90
    je int15_noop

    cmp ah, 0x91
    je int15_noop

    cmp ah, 0xC0
    je int15_c0

    cmp ah, 0x41                ; Wait On External Event
    je .unimpl

    cmp ah, 0x86                ; Elapsed Time Wait
    je .unimpl

    cmp ah, 0x88                ; Extended Memory Size Determination
    je int15_88

    cmp ah, 0xC1                ; Return Extended BIOS Data Area Segment
    je int15_c1

    push si
    push ax
    mov si, unimpl15            ; uintentionally unimplemented functions will print a message
    call puts

    mov al, ah
    call puthex8
    pop ax
    pop si

    .unimpl:                    ; intentionally unimplemented function

    ; set carry flag, unimplemented function
    mov ah, 0x86

    push bp
    mov bp, sp
    or byte [bp + 6], F_CF  ; skip bp, ip, cs in stack to get to flags
    pop bp
    iret

int15_4f:
    ; AH = 0x4F - Keyboard Intercept
    ; default is no-op, user code can hook to perform custom interception
    iret

int15_noop:
    push bp
    mov bp, sp
    and byte [bp + 6], ~F_CF  ; clear CF
    pop bp

    iret

int15_88:
    ; AH = 0x88 - Extended Memory Size Determination
    ; No EMS.
    xor ax, ax

    push bp
    mov bp, sp
    and byte [bp + 6], ~F_CF  ; clear CF
    pop bp

    iret

int15_c0:
    ; AH = 0xC0 - Get System Configuration
    mov ax, cs
    mov es, ax
    mov bx, sysconfig
    iret

int15_c1:
    ; AH = 0xC1 - Return Extended BIOS Data Area Segment
    push ax
    push cx
    push ds
    mov ax, 0x40                    ; BDA segment
    mov ds, ax
    mov ax, [ds:0x13]               ; memory size in KB, minus 4K
    mov cx, 6
    shl ax, cl                      ; turn KB count into a segment
    mov es, ax
    pop ds
    pop cx
    pop ax
    iret

int16:
    ; INT 16 - Keyboard Services

    call kbc_sync_flags_leds        ; sync keyboard flags and LEDs on entry to int16

    cmp ah, 0x00
    je int16_00

    cmp ah, 0x01
    je int16_01

    cmp ah, 0x02
    je int16_02

    cmp ah, 0x10
    je int16_00

    cmp ah, 0x11
    je int16_01

    cmp ah, 0x12
    je int16_12

    cmp ah, 0x55
    je .unimpl

    push ax
    push si

    mov si, unimpl16            ; uintentionally unimplemented functions will print a message
    call puts

    mov al, ah
    call puthex8

    pop si
    pop ax

    cli
    hlt

    .unimpl:                    ; intentionally unimplemented function

    iret

int16_00:
    ; AH = 0x00 - Read Next Character
    ; blocks until the character is read
    push ds
    push si

    mov ax, 0x40
    mov ds, ax

    sti

    .loop:
    mov si, [ds:0x1A]           ; head
    cmp si, [ds:0x1C]           ; head == tail?
    jz .loop                    ; yes, retry

    cli                         ; avoid buffer modifications while we edit the buffer

    mov ax, [ds:si]             ; character

    add si, 2                   ; advance head
    cmp si, 0x3E                ; check for wrap (buffer is 0x1E -> 0x3E)
    jne .done
    mov si, 0x1E                ; wrap back to start of KB buffer
    .done:
    mov word [ds:0x1A], si      ; update head

    pop si
    pop ds
    iret

int16_01:
    push bp
    mov bp, sp
    push ds
    push si

    ; AH = 0x01 - Report If Character Ready
    ; returns immediately

    mov ax, 0x40
    mov ds, ax

    sti

    mov si, [ds:0x1A]           ; head
    cmp si, [ds:0x1C]           ; tail
    jz .no_key

    ; SI = offset from 40:00 to head of buffer (FIFO)
    mov ax, word [ds:si]

    and word [bp + 6], ~F_ZF ; clear ZF, character ready - skip bp, ip, cs in stack to get to flags

    jmp .done

    .no_key:

    xor ax, ax

    or word [bp + 6], F_ZF ; set ZF, character not ready - skip bp, ip, cs in stack to get to flags

    .done:
    pop si
    pop ds
    pop bp
    iret

int16_02:
    ; AH = 0x02 - Get Shift Status
    push ds
    mov ax, 0x40
    mov ds, ax
    mov al, byte [ds:0x17]   ; Keyboard Status flags
    pop ds
    iret

int16_12:
    ; AH = 0x12 - Extended Get Keyboard Status
    push ds
    mov ax, 0x40
    mov ds, ax
    mov al, byte [ds:0x17]
    mov ah, byte [ds:0x18]
    pop ds
    iret

int17:
    ; INT 17 - Printer Services

    cmp ah, 0x00
    je int17_00

    cmp ah, 0x01
    je int17_01

    cmp ah, 0x02
    je int17_02

    push si
    mov si, unimpl17
    call puts
    pop si

    iret

int17_00:
    push si

    ; AH = 0x00 - Send Byte to Printer
    ; AL = character to be printed
    ; DX = printer number
    shl dx, 1
    mov si, dx
    add si, parallelports
    mov dx, [cs:si]

    push ax

    .waitready:
    in al, dx               ; ensure printer is not busy before we transmit
    test al, 0x80
    jz .waitready

    pop ax

    out dx, al              ; transmit data

    add dx, 2               ; Control register

    in al, dx

    or al, 0x01             ; STROBE
    out dx, al

    mov ax, 0x01            ; approx. 10ms hold

    and al, ~0x01           ; clear STROBE
    out dx, al

    sub dx, 1               ; Status register

    .txwait:
    in al, dx               ; read status register
    test al, 0x80
    jz .txwait

    mov ah, al              ; status returned in AH

    pop si
    iret

int17_01:
    push si

    ; AH = 0x01 - Initialize Printer
    ; DX = printer number
    shl dx, 1
    mov si, dx
    add si, parallelports
    mov dx, [cs:si]

    add dx, 2               ; Control register

    xor al, al
    out dx, al              ; INITIALIZE is active low

    pop si
    iret

int17_02:
    push si

    ; AH = 0x02 - Get Printer Status
    ; DX = printer number
    shl dx, 1
    mov si, dx
    add si, parallelports
    mov dx, [cs:si]

    add dx, 1               ; Status register

    in al, dx               ; Return status register

    pop si
    iret

int19:
    ; INT 19 - Bootstrap Loader
    mov si, noint19
    call puts

    sti
    jmp $

    iret

int1a:
    ; INT 1A - Time Services

    cmp ah, 0x00
    je int1a_00

    cmp ah, 0x01
    je int1a_01

    ; set CF, unimplemented
    push bp
    mov bp, sp
    or word [bp + 6], F_CF ; set CF, unimplemented - skip bp, ip, cs in stack to get to flags
    pop bp

    iret

int1a_00:
    ; AH = 0x00 - Get System Clock Counter
    push ds
    mov ax, 0x40
    mov ds, ax
    mov al, byte [ds:0x70]  ; clock rollover flag
    mov dx, [ds:0x6C]       ; ticks since midnight, low word
    mov cx, [ds:0x6E]       ; ticks since midnight, high word
    pop ds
    iret

int1a_01:
    ; AH = 0x01 - Set System Clock Counter
    push ds
    mov ax, 0x40
    mov ds, ax
    mov [ds:0x6C], dx       ; ticks since midnight, low word
    mov [ds:0x6E], cx       ; ticks since midnight, high word
    pop ds
    ret

segment _DATA public align=16 use16 class=DATA

unimpl13 db "Dummy INT 13h called for some reason", 13, 10, 0
unimpl14 db "Unimplemented INT 14h function", 13, 10, 0
unimpl15 db "Unimplemented INT 15h function", 13, 10, 0
unimpl16 db "Unimplemented INT 16h function", 13, 10, 0
unimpl17 db "Unimplemented INT 17h function", 13, 10, 0

noint19 db "No INT 19 handler, install an Option ROM", 13, 10, 0

serialbios db "Serial Port BIOS Services", 13, 10, 0

sysconfig:
    dw 0x08         ; length
    db 0xFE         ; model
    db 0x00         ; submodel
    db 0x00         ; BIOS revision
    db (1 << 6) | (1 << 2)     ; feature byte - EBDA, two PICs
    dd 0            ; reserved

parallelports:
    dw 0x0378       ; LPT1
    dw 0x0278       ; LPT2
    dw 0x03BC       ; LPT3

serialports:
    dw 0x03F8       ; COM1
    dw 0x02F8       ; COM2
    dw 0x03E8       ; COM3
    dw 0x02E8       ; COM4

serialdivisors:
    db 23, 4       ; 110 baud (divisor = 1047)
    db 0, 3        ; 150 baud (divisor = 768)
    db 128, 1      ; 300 baud (divisor = 384)
    db 192, 0      ; 600 baud
    db 96, 0       ; 1200 baud
    db 48, 0       ; 2400 baud
    db 24, 0       ; 4800 baud
    db 12, 0       ; 9600 baud
