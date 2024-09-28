bits 16
cpu 8086

section .text

global configure_i2c
global i2c_send_byte
global i2c_send_str

PCF8584_LO              equ 0x2A0
PCF8584_HI              equ 0x2A1

; S1 register (Read - Status)
PCF8584_BIT_STS_BB     equ 0
PCF8584_BIT_STS_LAB    equ 1
PCF8584_BIT_STS_AAS    equ 2
PCF8584_BIT_STS_LRB    equ 3
PCF8584_BIT_STS_BER    equ 4
PCF8584_BIT_STS_STS    equ 5
PCF8584_BIT_STS_PIN    equ 7

PCF8584_MASK_STS_BB    equ (1 << PCF8584_BIT_STS_BB)
PCF8584_MASK_STS_LAB   equ (1 << PCF8584_BIT_STS_LAB)
PCF8584_MASK_STS_AAS   equ (1 << PCF8584_BIT_STS_AAS)
PCF8584_MASK_STS_LRB   equ (1 << PCF8584_BIT_STS_LRB)
PCF8584_MASK_STS_BER   equ (1 << PCF8584_BIT_STS_BER)
PCF8584_MASK_STS_STS   equ (1 << PCF8584_BIT_STS_STS)
PCF8584_MASK_STS_PIN   equ (1 << PCF8584_BIT_STS_PIN)

; S1 register (Write - Control)
PCF8584_BIT_CTL_ACK    equ 0
PCF8584_BIT_CTL_STO    equ 1
PCF8584_BIT_CTL_STA    equ 2
PCF8584_BIT_CTL_ENI    equ 3
PCF8584_BIT_CTL_ES     equ 4
PCF8584_BIT_CTL_ESO    equ 6
PCF8584_BIT_CTL_PIN    equ 7

PCF8584_MASK_CTL_ACK    equ (1 << PCF8584_BIT_CTL_ACK)
PCF8584_MASK_CTL_STO    equ (1 << PCF8584_BIT_CTL_STO)
PCF8584_MASK_CTL_STA    equ (1 << PCF8584_BIT_CTL_STA)
PCF8584_MASK_CTL_ENI    equ (1 << PCF8584_BIT_CTL_ENI)
PCF8584_MASK_CTL_ES     equ (3 << PCF8584_BIT_CTL_ES)
PCF8584_MASK_CTL_ES1    equ (1 << PCF8584_BIT_CTL_ES)
PCF8584_MASK_CTL_ES2    equ (2 << PCF8584_BIT_CTL_ES)
PCF8584_MASK_CTL_ESO    equ (1 << PCF8584_BIT_CTL_ESO)
PCF8584_MASK_CTL_PIN    equ (1 << PCF8584_BIT_CTL_PIN)

; S2 register
PCF8584_SCL_90K         equ (0 << 0)
PCF8584_SCL_45K         equ (1 << 0)
PCF8584_SCL_11K         equ (2 << 0)
PCF8584_SCL_1_5K        equ (3 << 0)

PCF8584_CLK_3MHZ        equ (0 << 2)
PCF8584_CLK_4_43MHZ     equ (4 << 2)
PCF8584_CLK_6MHZ        equ (5 << 2)
PCF8584_CLK_8MHZ        equ (6 << 2)
PCF8584_CLK_12MHZ       equ (7 << 2)

; at 4.77 MHz we need 3-6 cycles between each IO,
; a single nop should do the trick!

configure_i2c:
    push ax
    push dx

    mov dx, PCF8584_HI              ; 80H, disable I2C bus, prepare to load address to S0
    mov al, PCF8584_MASK_CTL_PIN
    out dx, al

    nop

    mov dx, PCF8584_LO              ; configure our address as 0xAA
    mov al, 0xAA
    shr al, 1
    out dx, al

    nop

    mov dx, PCF8584_HI              ; prepare to load clock register (S2)
    mov al, PCF8584_MASK_CTL_PIN | PCF8584_MASK_CTL_ES2
    out dx, al

    nop

    mov dx, PCF8584_LO              ; ~4.43 MHz, 90 kHz SCL
    mov al, PCF8584_CLK_4_43MHZ | PCF8584_SCL_90K
    out dx, al

    nop

    mov dx, PCF8584_HI              ; enable I2C bus, go idle
    mov al, PCF8584_MASK_CTL_PIN | PCF8584_MASK_CTL_ESO | PCF8584_MASK_CTL_ACK
    out dx, al

    nop

    pop dx
    pop ax
    ret

; AL = I2C target address
; BL = byte to send
i2c_send_byte:
    push ax
    push dx
    push bx
    push cx

    push ax
    call i2c_wait_idle
    pop ax

    mov dx, PCF8584_LO
    shl al, 1
    out dx, al

    nop

    mov dx, PCF8584_HI              ; I2C "START"
    mov al, PCF8584_MASK_CTL_ACK | PCF8584_MASK_CTL_STA | PCF8584_MASK_CTL_ESO | PCF8584_MASK_CTL_PIN
    out dx, al

    nop

    mov cx, 2
    .l:

    call i2c_wait_tx_finish
    test al, PCF8584_MASK_STS_LRB
    jnz .fail                       ; target did not acknowledge

    sub cx, 1
    jz .out

    mov dx, PCF8584_LO
    mov al, bl
    out dx, al

    nop
    jmp .l

    .out:

    mov bh, 0
    jmp .done

    .fail:
    mov bh, 1

    .done:

    mov dx, PCF8584_HI              ; I2C "STOP"
    mov al, PCF8584_MASK_CTL_ACK | PCF8584_MASK_CTL_STO | PCF8584_MASK_CTL_ESO | PCF8584_MASK_CTL_PIN
    out dx, al

    mov ah, bh                      ; exit status (0=ok, 1=fail)
    pop cx
    pop bx
    pop dx
    pop ax
    ret

; AL = I2C target address
; BX = byte count
; DS:SI = buffer
i2c_send_str:
    push ax
    push dx
    push bx
    push cx

    push ax
    call i2c_wait_idle
    pop ax

    mov dx, PCF8584_LO
    shl al, 1
    out dx, al

    nop

    mov dx, PCF8584_HI              ; I2C "START"
    mov al, PCF8584_MASK_CTL_ACK | PCF8584_MASK_CTL_STA | PCF8584_MASK_CTL_ESO | PCF8584_MASK_CTL_PIN
    out dx, al

    nop

    mov cx, bx
    add cx, 1                       ; one extra cycle for final ACK
    .l:

    call i2c_wait_tx_finish
    test al, PCF8584_MASK_STS_LRB
    jnz .fail                       ; target did not acknowledge

    sub cx, 1
    jz .out

    mov dx, PCF8584_LO
    mov al, byte [ds:si]
    out dx, al

    inc si

    nop
    jmp .l

    .out:

    mov bh, 0
    jmp .done

    .fail:
    mov bh, 1

    .done:

    mov dx, PCF8584_HI              ; I2C "STOP"
    mov al, PCF8584_MASK_CTL_ACK | PCF8584_MASK_CTL_STO | PCF8584_MASK_CTL_ESO | PCF8584_MASK_CTL_PIN
    out dx, al

    mov ah, bh                      ; exit status (0=ok, 1=fail)
    pop cx
    pop bx
    pop dx
    pop ax
    ret

; AL = status register after completion
i2c_wait_idle:
    mov dx, PCF8584_HI
    .l:
    in al, dx
    nop
    test al,PCF8584_MASK_STS_BB
    jz .l                           ; if BB=0, bus busy
    ret

; AL = status register after completion
i2c_wait_tx_finish:
    mov dx, PCF8584_HI
    .l:
    in al, dx
    nop
    test al,PCF8584_MASK_STS_PIN
    jnz .l                          ; if PIN=1, transmit not complete
    ret
