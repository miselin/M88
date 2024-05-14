bits 16
cpu 8086

segment _TEXT public align=16 use16 class=CODE

global configure_serial

configure_serial:
    mov ax, 0x40
    mov ds, ax

    .port1:
    mov dx, 0x3F8
    call configure_port
    jc .port2
    mov [ds:0x00], dx

    .port2:
    mov dx, 0x2F8
    call configure_port
    jc .port3
    mov [ds:0x02], dx

    .port3:
    mov dx, 0x3E8
    call configure_port
    jc .port4
    mov [ds:0x04], dx

    .port4:
    mov dx, 0x2E8
    call configure_port
    jc .done
    mov [ds:0x06], dx

    .done:
    ret

configure_port:
    add dx, 1                   ; disable all interrupts
    mov al, 0x00
    out dx, al
    add dx, 2
    mov al, 0x80
    out dx, al                ; enable DLAB
    sub dx, 3
    mov al, 0x03
    out dx, al                ; set divisor to 3 (38400 baud)
    add dx, 1
    mov al, 0x00
    out dx, al                ; hi byte of divisor
    add dx, 2
    mov al, 0x03
    out dx, al                ; 8 bits, no parity, one stop bit
    sub dx, 1
    mov al, 0xC7
    out dx, al                ; enable FIFO, clear it, set a 14-byte threshold for IRQ
    add dx, 2
    mov al, 0x0B
    out dx, al                ; RTS/DSR set
    mov al, 0x1E
    out dx, al                ; do a looopback test
    sub dx, 4
    mov al, 0xAE
    out dx, al                ; send test message
    in al, dx
    cmp al, 0xAE
    jne .faulty

    add dx, 4
    mov al, 0x0F
    out dx, al                ; back to normal function
    sub dx, 4                   ; back to base port

    clc
    ret

    .faulty:
    stc
    ret
