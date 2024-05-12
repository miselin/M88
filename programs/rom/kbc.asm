bits 16
cpu 8086

segment _TEXT public align=16 use16 class=CODE

extern unmask_irq
extern puts
extern delay_ticks
extern _handle_scancode
extern toupper

global configure_kbc
global kbirq

extern _kbascii
extern _kbascii_shift
extern _kbascii_ctrl
extern _kbascii_alt
extern kbnumpad
extern kbnumpad_numlock

; TODO: this file needs timeouts!

kbc_wait_write:
    out 0xE0, al
    in al, 0x64
    test al, 2
    jnz kbc_wait_write
    ret

kbc_wait_read:
    in al, 0x64
    test al, 1
    jz kbc_wait_read
    ret

; write a ps/2 controller command
kbc_write_cmd:
    push ax
    call kbc_wait_write
    pop ax

    out 0x64, al
    ret

; read a byte from the ps/2 controller
kbc_read_cmd:
    call kbc_wait_read

    in al, 0x64
    ret

; write the byte in al to the first device
kbc_write_dev0:
    push ax
    call kbc_wait_write
    pop ax

    out 0x60, al
    ret

; read a byte from the first device, return in al
kbc_read_dev0:
    call kbc_wait_read

    in al, 0x60
    ret

; read a byte from the first device, without blocking
; sets CF if no byte is available
kbc_read_dev0_nb:
    in al, 0x64
    test al, 1
    jz .no_byte

    in al, 0x60
    clc
    ret

    .no_byte:
    stc
    ret

; flush the KBC's output buffer
kbc_flush_obf:
    in al, 0x64
    out 0xE0, al
    test al, 1
    jz .done
    in al, 0x00
    jmp kbc_flush_obf
    .done:
    ret

kbc_clearbuf_dev0:
    clc
    call kbc_read_dev0_nb
    jnc kbc_clearbuf_dev0
    ret

extern putc
extern puthex8

configure_kbc:
    cli

    ; disable all devices
    mov al, 0xAD
    call kbc_write_cmd

    mov al, 0xA7
    call kbc_write_cmd

    ; disable IRQs
    mov al, 0x20
    call kbc_write_cmd
    call kbc_read_dev0
    and al, ~3
    or al, 0x40

    mov bx, ax
    mov al, 0x60
    call kbc_write_cmd
    mov al, bl
    call kbc_write_dev0

    ; self-test the PS/2 controller
    mov al, 0xAA
    call kbc_write_cmd
    call kbc_read_dev0
    cmp al, 0x55
    jne .fail

    ; enable first port
    mov al, 0xAE
    call kbc_write_cmd

    call kbc_clearbuf_dev0          ; clear out any leftover data from init

    ; reset devices
    .resend:
    mov al, 'G'
    call putc
    mov al, 0xFF
    call kbc_write_dev0
    call kbc_read_dev0              ; ack
    cmp al, 0xFE
    je .resend

    call puthex8

    cmp al, 0xAA                    ; did we have a leftover BAT from powerup?
    jne .nobat

    call kbc_read_dev0              ; now read next which should be ACK from reset

    .nobat:

    cmp al, 0xFA
    jne .fail

    call kbc_read_dev0 ; self-test result
    cmp al, 0xAA
    jne .fail

    ; turn on num lock
    mov al, 0xED
    call kbc_write_dev0
    mov al, 0x02
    call kbc_write_dev0

    ; consume the ACK to ensure the buffer is clear
    call kbc_read_dev0

    mov ax, 2                       ; hold for a bit to let things settle before
    call delay_ticks                ; we clear the buffer and enable IRQs

    .clearbuf:                      ; clear the KB buffer to avoid stray IRQs
    clc                             ; sometimes we get multiple ACKs and those would
    call kbc_read_dev0_nb           ; fire an unwanted IRQ when we enable it below
    jnc .clearbuf

    mov ax, 1                       ; unmask IRQ1
    call unmask_irq

    mov al, 0x20                    ; enable KB IRQs
    call kbc_write_cmd
    call kbc_read_dev0
    or al, 3
    mov bx, ax
    mov al, 0x60
    call kbc_write_cmd
    mov al, bl
    call kbc_write_dev0

    ; reset BDA keyboard areas
    mov ax, 0x40
    mov es, ax
    xor di, di

    mov byte [es:0x17], 0           ; kb flag byte 0
    mov byte [es:0x18], 0           ; kb flag byte 1
    mov byte [es:0x1A], 0x1E        ; kb buffer head
    mov byte [es:0x1C], 0x1E        ; kb buffer tail
    mov byte [es:0x96], 0b00010000  ; kb mode/type (101/102 enhanced)
    mov byte [es:0x97], 0           ; kb led flags

    sti
    ret

    .fail:
    mov si, kbfail
    call puts

    sti
    ret

kbirq:
    cli

    push ax
    push bx
    push cx
    push dx
    push si
    push di
    push bp
    push ds
    push es

    ; C code expects DS=ES and data segment is the ROM segment
    mov ax, 0x40
    mov ds, ax

    mov ax, cs
    mov es, ax

    call kbc_read_dev0              ; scancode in AL

    mov ah, al                      ; raw scancode in AH

    cmp al, 0xE0                    ; extended scancode?
    jne .not_extended

    or byte [ds:0x96], 2            ; set "last key E0" flag
    jmp .done

    .not_extended:
    test byte [ds:0x96], 2          ; was the last key an extended key?
    jnz .extended

    push ax
    call check_special
    pop ax
    jc .done                        ; sets carry flag if special key was handled

    test al, 0x80                   ; key up?
    jnz .done                       ; ignore key up events (special key handler uses them though)

    and al, 0x7F                    ; clear key up bit

    cmp al, 0x46                    ; ESC-ScrollLock
    jae .numpad                     ; ignore keys outside the range for now

    mov ah, al                      ; preserve raw scancode

    test byte [ds:0x17], 0x3        ; either shift key down?
    jz .noshift
    mov bx, _kbascii_shift
    jmp .lookup
    .noshift:

    test byte [ds:0x18], 0x4        ; either control key down?
    jz .noctrl
    mov bx, _kbascii_ctrl
    jmp .lookup
    .noctrl:

    test byte [ds:0x18], 0x8        ; either alt key down?
    jz .noalt
    mov bx, _kbascii_alt
    jmp .lookup
    .noalt:

    mov bx, _kbascii

    .lookup:
    es xlat                         ; translate scancode to ASCII (use ES segment which is ROM data)

    test byte [ds:0x97], 4          ; caps lock?
    jz .nocaps
    call toupper                    ; yes, convert to uppercase
    .nocaps:

    call kb_put_buffer              ; put into kb circular buffer

    jmp .done

    .extended:                      ; do nothing for extended keys for now

    and byte [ds:0x96], ~2          ; remove "last key E0" flag

    jmp .done

    .numpad:

    mov ah, al                      ; preserve raw scancode
    sub al, 0x47                    ; prepare for numpad lookup

    mov bx, kbnumpad                ; load default numpad table
    test byte [ds:0x97], 2          ; num lock?
    jz .nonumlock
    mov bx, kbnumpad_numlock        ; load num-locked numpad table
    .nonumlock:                     ; TODO: CTRL-, SHIFT-, ALT- tables

    es xlat
    call kb_put_buffer

    .done:

    mov al, 0x20                    ; EOI
    out 0x20, al

    pop es
    pop ds
    pop bp
    pop di
    pop si
    pop dx
    pop cx
    pop bx
    pop ax

    iret

; AH = scancode, AL = ASCII
; DS assumed to be 0x40
kb_put_buffer:
    mov si, [ds:0x1C]               ; buffer tail
    mov [ds:si], ax                 ; put key in buffer
    add si, 2
    cmp si, 0x3E                    ; buffer end?
    jb .finish
    mov si, 0x1E                    ; wrap around
    .finish:
    mov [ds:0x1C], si               ; update buffer tail
    ret

check_special:
    and al, 0x7F                    ; clear key up bit

    .lctrl:
    cmp al, 0x1D                    ; Left Ctrl
    jne .lshift

    test ah, 0x80
    jnz .lctrl_up
    or byte [ds:0x18], 1            ; set Left Ctrl flag
    jmp .done
    .lctrl_up:
    and byte [ds:0x18], ~1          ; clear Left Ctrl flag
    jmp .done

    .lshift:
    cmp al, 0x2A
    jne .rshift

    test ah, 0x80
    jnz .lshift_up
    or byte [ds:0x17], 2            ; set Left Shift flag
    jmp .done
    .lshift_up:
    and byte [ds:0x17], ~2          ; clear Left Shift flag
    jmp .done

    .rshift:
    cmp al, 0x36
    jne .lalt

    test ah, 0x80
    jnz .rshift_up
    or byte [ds:0x17], 1            ; set Right Shift flag
    jmp .done
    .rshift_up:
    and byte [ds:0x17], ~1          ; clear Right Shift flag
    jmp .done

    .lalt:
    cmp al, 0x38
    jne .capslock

    test ah, 0x80
    jnz .lalt_up
    or byte [ds:0x18], 2            ; set Left Alt flag
    jmp .done
    .lalt_up:
    and byte [ds:0x18], ~2          ; clear Left Alt flag
    jmp .done

    .capslock:
    cmp al, 0x3A
    jne .numlock
    test ah, 0x80                   ; only handle lock keys on keyup
    jz .done
    xor byte [ds:0x97], 4           ; toggle caps lock bit
    jmp .done

    .numlock:
    cmp al, 0x45
    jne .scrolllock
    test ah, 0x80                   ; only handle lock keys on keyup
    jz .done
    xor byte [ds:0x97], 2           ; toggle num lock bit
    jmp .done

    .scrolllock:
    cmp al, 0x46
    jne .not_special
    test ah, 0x80                   ; only handle lock keys on keyup
    jz .done
    xor byte [ds:0x97], 1           ; toggle scroll lock bit

    .done:

    and byte [ds:0x17], ~0xC        ; mask out CTRL/ALT flags
    mov al, [ds:0x96]               ; grab RCTRL/RALT flags
    and al, 0x0C
    or byte [ds:0x17], al           ; store into CTRL/ALT flags
    mov al, [ds:0x18]               ; grab LCTRL/LALT flags
    and al, 0x3
    shl al, 1                       ; bits 0/1 -> bits 2/3
    shl al, 1
    or byte [ds:0x17], al           ; merge into CTRL/ALT flags

    mov bl, byte [ds:0x97]          ; update LEDs on keyboard
    mov al, 0xED                    ; set LEDs command
    call kbc_write_dev0
    mov al, bl
    and al, 7                       ; only low 3 bits are for LEDs
    call kbc_write_dev0             ; send LED state

    call kbc_read_dev0              ; consume the ACK

    mov bl, byte [ds:0x97]          ; update LEDs in lower keyboard flags byte
    mov cx, 4
    and bl, 7
    shl bl, cl
    or byte [ds:0x17], bl           ; bits 4-6 are the LED state

    stc
    ret

    .not_special:
    clc
    ret



segment _DATA public align=16 use16 class=DATA

kbstr db "Keyboard IRQ", 13, 10, 0

kbfail db "Keyboard initialization failed", 13, 10, 0
