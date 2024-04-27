bits 16
cpu 8086

segment code public align=16 use16 class=code

global configure_fdc
global fdc_irq
global fdc_read_drive0
global fdc_motor_off
extern delay_ticks
extern set_fdc_shutoff_counter
extern prepare_dma2_read

FDC_STATUS_REGISTER_A           equ 0x3F0
FDC_STATUS_REGISTER_B           equ 0x3F1
FDC_DIGITAL_OUTPUT_REGISTER     equ 0x3F2
FDC_TAPE_DRIVE_REGISTER         equ 0x3F3
FDC_MAIN_STATUS_REGISTER        equ 0x3F4       ; when read
FDC_DATARATE_SELECT_REGISTER    equ 0x3F4       ; when written
FDC_DATA_FIFO                   equ 0x3F5
FDC_DIGITAL_INPUT_REGISTER      equ 0x3F7
FDC_CONFIG_CONTROL_REGISTER     equ 0x3F7

fdc_reset:
    mov al, 0x00                                ; reset FDC
    mov dx, FDC_DIGITAL_OUTPUT_REGISTER
    out dx, al

    mov ax, 1                                   ; hold RESET low for ~50 ms
    call delay_ticks

    mov al, 0x0C                                ; release reset bit, enable DMA
    out dx, al

    mov ax, 2                                   ; hold for ~100 ms, rising edge of RESET will reset FDC
    call delay_ticks

    ; wait for FDC to be ready
    .wait_ready:
    mov ax, 1
    call delay_ticks

    mov dx, FDC_MAIN_STATUS_REGISTER
    in al, dx
    and al, 0xC0
    cmp al, 0x80
    jnz .wait_ready

    call fdc_sense_interrupt                    ; run sense interrupt 4 times now that reset is complete
    call fdc_sense_interrupt
    call fdc_sense_interrupt
    call fdc_sense_interrupt

    mov dx, FDC_CONFIG_CONTROL_REGISTER
    mov al, 0x00
    out dx, al                                  ; set default data rate (500 kbps)

    ret

; run the sense interrupt command, returns ST0 in AX and the cylinder # in DX
fdc_sense_interrupt:
    mov ax, 0x08
    call fdc_start_command

    call fdc_fifo_read_wait
    call fdc_read_fifo      ; st0
    push ax
    call fdc_read_fifo      ; cyl
    mov dx, ax
    pop ax
    ret

; send command to FDC
fdc_start_command:
    push dx
    push ax

    .wait_ready:
    ; wait for FDC to be ready
    mov dx, FDC_MAIN_STATUS_REGISTER
    in al, dx
    and al, 0xC0
    cmp al, 0x80
    je .ok

    call fdc_reset
    jmp .wait_ready

    .ok:

    pop ax
    mov dx, FDC_DATA_FIFO
    out dx, al

    pop dx
    ret

; send a parameter to the FDC
fdc_send_parameter:
    push dx
    push ax

    .wait_ready:
    mov dx, FDC_MAIN_STATUS_REGISTER
    in al, dx

    test al, 0x80       ; check RQM=1, host can transfer data
    jz .wait_ready

    test al, 0x40       ; check DIO=0, FDC is expecting a write
    jnz .wait_ready

    pop ax
    mov dx, FDC_DATA_FIFO
    out dx, al

    pop dx
    ret

; wait for the FDC to have data to read from the FIFO
fdc_fifo_read_wait:
    push dx
    push ax

    .wait_ready:
    mov ax, 1
    call delay_ticks

    mov dx, FDC_MAIN_STATUS_REGISTER
    in al, dx
    test al, 0x80       ; check RQM=1, host can transfer data
    jz .wait_ready

    test al, 0x40       ; check DIO=1, FDC is expecting a read
    jz .wait_ready

    pop ax
    pop dx
    ret

; wait for the FDC to be ready for more data from the host
fdc_fifo_write_wait:
    push dx
    push ax

    .wait_ready:
    mov ax, 1
    call delay_ticks

    mov dx, FDC_MAIN_STATUS_REGISTER
    in al, dx
    test al, 0x80       ; check RQM=1, host can transfer data
    jz .wait_ready

    test al, 0x40       ; check DIO=0, FDC is expecting a write
    jnz .wait_ready

    pop ax
    pop dx
    ret

; wait for an FDC IRQ
fdc_wait_irq:
    push ax
    push es

    mov ax, 0x40
    mov es, ax

    cli                         ; avoid stray interrupts while we set up

    mov al, byte [es:0x3E]      ; load recalibration status byte
    and al, 0x7F                ; mask out the IRQ bit
    mov byte [es:0x3E], al

    .wait_irq:
    sti                         ; setup complete, allow interrupts once again
    mov al, byte [es:0x3E]      ; read the status byte
    test al, 0x80               ; check the IRQ bit
    jnz .done

    mov ax, 1                  ; wait ~50 ms before next check
    call delay_ticks
    jmp .wait_irq               ; loop until it's set

    .done:

    pop es
    pop ax
    ret

fdc_wait_complete:
    push ax
    push dx

    ; wait for FDC to be ready
    .wait_ready:
    mov ax, 1
    call delay_ticks

    mov dx, FDC_MAIN_STATUS_REGISTER
    in al, dx
    test al, 0x10

    jnz .wait_ready

    pop dx
    pop ax
    ret

fdc_read_fifo:
    push dx
    call fdc_fifo_read_wait
    mov dx, FDC_DATA_FIFO
    in al, dx
    pop dx
    ret

configure_fdc:
    push ax

    ; unmask IRQ6
    in al, 0x21
    and al, 0b10111111
    out 0x21, al

    jmp .just_sense

    call fdc_reset

    mov ax, 0x10            ; Version command
    call fdc_start_command
    call fdc_fifo_read_wait

    call fdc_read_fifo
    cmp al, 0x90            ; 0x90 = enhanced controller, skip FDC if not
    jne .error

    mov ax, 0x13            ; Configure command
    call fdc_start_command
    mov ax, 0x00
    call fdc_send_parameter
    mov ax, 0x57            ; implied seek, FIFO enabled, polling mode off, 8 bytes threshold
    call fdc_send_parameter
    mov ax, 0x00            ; no precompensation
    call fdc_send_parameter

    mov ax, 0x03            ; Specify command
    call fdc_start_command
    mov ax, 0x80            ; SRT = 8, HUT = 0
    call fdc_send_parameter
    mov ax, 0x0A            ; HLT = 5, ND = 0 (enable DMA)
    call fdc_send_parameter

    .just_sense:

    call fdc_motor_on       ; turn on drive 0 motor

    mov ax, 36              ; let it spin up
    call delay_ticks

    mov ax, 0x07            ; Recalibrate command
    call fdc_start_command
    mov ax, 0               ; Recalibrate drive 0
    call fdc_send_parameter
    call fdc_wait_irq

    call fdc_sense_interrupt
    cmp ax, 0x20            ; ST0 = 0x20 is OK
    jne .error

    pop ax
    ret

    .error:
    cli
    hlt

fdc_irq:
    push ax
    push es

    mov al, 'F'
    out 0xE9, al

    mov ax, 0x40
    mov es, ax

    or byte [es:0x3E], 0x80

    pop es
    pop ax
    ret

; ES:DX = buffer to read into
; CX = number of bytes to read
; BX = cylinder number
; Returns AX=0 on success
fdc_read_drive0:
    push es
    push dx
    push cx

    ; mov ax, 0x0F            ; Seek command
    ; call fdc_start_command
    ; mov ax, 0x00
    ; call fdc_send_parameter               ; Head 0, Drive 0
    ; mov ax, bx                          ; Cylinder
    ; call fdc_send_parameter
    ; call fdc_wait_irq

    ; call fdc_sense_interrupt

    pop cx
    pop dx
    pop es

    ; configure DMA for reading
    call prepare_dma2_read

    ; kick start the motor
    call fdc_motor_on

    ; keep the motor on for a long time
    mov ax, 0xFFFF
    call set_fdc_shutoff_counter

    ; kick off a read
    mov ax, 0xE6                        ; CMD: MT | MFM | SKIP | Read Data
    call fdc_start_command

    mov ax, 0x00                        ; HDS, drive select
    call fdc_send_parameter
    mov ax, bx                          ; C: Cylinder
    call fdc_send_parameter
    mov ax, 0x00                        ; H: Head 0
    call fdc_send_parameter
    mov ax, 0x01                        ; S: Sector 1
    call fdc_send_parameter
    mov ax, 0x02                        ; N: 512 bytes per sector
    call fdc_send_parameter
    mov ax, 1                           ; EOT: 1 sector
    call fdc_send_parameter
    mov ax, 0x1B                        ; GPL: GAP1 default size
    call fdc_send_parameter
    mov ax, 0xFF                        ; DTL: 512 byte sectors, no special size
    call fdc_send_parameter

    mov dx, FDC_MAIN_STATUS_REGISTER
    in al, dx
    mov dx, FDC_STATUS_REGISTER_A
    in al, dx
    mov dx, FDC_STATUS_REGISTER_B
    in al, dx

    call fdc_wait_irq                   ; wait for DMA to complete

    call fdc_fifo_read_wait
    call fdc_read_fifo      ; st0
    call fdc_fifo_read_wait
    call fdc_read_fifo      ; st1
    call fdc_fifo_read_wait
    call fdc_read_fifo      ; st2
    call fdc_fifo_read_wait
    call fdc_read_fifo      ; cyl
    call fdc_fifo_read_wait
    call fdc_read_fifo      ; last head
    call fdc_fifo_read_wait
    call fdc_read_fifo      ; last sector
    call fdc_fifo_read_wait
    call fdc_read_fifo      ; 2

    mov ax, 36                          ; transfer complete, set motor shutoff to 2 seconds
    call set_fdc_shutoff_counter

    mov ax, 0
    ret

fdc_motor_off:
    push ax
    push dx
    mov ax, 0x0C                                ; no motor bits set
    mov dx, FDC_DIGITAL_OUTPUT_REGISTER
    out dx, al
    pop dx
    pop ax
    ret

fdc_motor_on:
    push ax
    push dx
    mov ax, 0x1C                                ; turn on drive 0 motor
    mov dx, FDC_DIGITAL_OUTPUT_REGISTER
    out dx, al

    mov ax, 36                                  ; set motor shutoff to 2 seconds
    call set_fdc_shutoff_counter

    pop dx
    pop ax
    ret
