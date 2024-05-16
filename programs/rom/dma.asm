bits 16
cpu 8086

section .text

global configure_dma
global prepare_dma2_write
global prepare_dma2_read
global test_dma

extern puts
extern puthex

DMA_START_ADDR_REG_0  equ 0x00      ; Start address register for channel 0
DMA_WORD_COUNT_REG_0  equ 0x01      ; Word count register for channel 0
DMA_START_ADDR_REG_1  equ 0x02      ; Start address register for channel 1
DMA_WORD_COUNT_REG_1  equ 0x03      ; Word count register for channel 1
DMA_START_ADDR_REG_2  equ 0x04      ; Start address register for channel 2
DMA_WORD_COUNT_REG_2  equ 0x05      ; Word count register for channel 2
DMA_START_ADDR_REG_3  equ 0x06      ; Start address register for channel 3
DMA_WORD_COUNT_REG_3  equ 0x07      ; Word count register for channel 3
DMA_STATUS_CMD_REG    equ 0x08      ; DMA command (W) and status (R) register
DMA_REQUEST_REG       equ 0x09      ; DMA request register
DMA_MASK_REG          equ 0x0A      ; DMA mask register
DMA_MODE_REG          equ 0x0B      ; DMA mode register
DMA_CLEAR_FF_REG      equ 0x0C      ; DMA clear byte pointer flip-flop "software command" (Fig. 6 in 8237A datasheet)
DMA_MASTER_CLEAR_REG  equ 0x0D      ; DMA master clear "software command"

DMA_PAGE_ADDR_REG_0   equ 0x87      ; Page register for channel 0
DMA_PAGE_ADDR_REG_1   equ 0x83      ; Page register for channel 1
DMA_PAGE_ADDR_REG_2   equ 0x81      ; Page register for channel 2
DMA_PAGE_ADDR_REG_3   equ 0x82      ; Page register for channel 3

DMA_MODE_REG_VERIFY   equ 0x00      ; Verify transfer
DMA_MODE_REG_WRITE    equ 0x04      ; Write transfer
DMA_MODE_REG_READ     equ 0x08      ; Read transfer
DMA_MODE_REG_AUTO     equ 0x10      ; Auto-init transfer
DMA_MODE_REG_ADDR_INC equ 0x00      ; Address increment
DMA_MODE_REG_SINGLE   equ 0x40      ; Single transfer
DMA_MODE_REG_DEMAND   equ 0x00      ; Demand transfer

configure_dma:
    push ax

    mov al, 0x00
    out DMA_MASTER_CLEAR_REG, al    ; DMA-1 software reset

    mov al, 0x00
    out 0xDA, al                    ; DMA-2 software reset

    mov al, 0xC0
    out 0xD8, al                    ; DMA-2 mode register (cascade)

    mov al, 0x00
    out 0xD4, al                    ; DMA-2 clear mask for channel 0 (cascade channel)

    pop ax
    ret

; Prepares a DMA transfer on channel 2, typically used for the FDC
; Pass a buffer address in ES:DX. ES must be aligned to a 4K boundary.
; Size of transfer should be in CX, and less than 64K
prepare_dma2:
    push ax
    push cx
    mov al, 0x06
    out DMA_MASK_REG, al            ; Mask channels 0 and 2 during configuration
    mov al, 0xFF
    out DMA_CLEAR_FF_REG, al        ; Reset DMA master flip-flop
    mov al, dl
    out DMA_START_ADDR_REG_2, al    ; Set low byte of address (0x7E00)
    mov al, dh
    out DMA_START_ADDR_REG_2, al    ; Set high byte of address (0x7E00)
    mov al, 0xFF
    out DMA_CLEAR_FF_REG, al        ; Reset DMA master flip-flop
    sub cx, 1                       ; Count is one less than actual size
    mov al, cl
    out DMA_WORD_COUNT_REG_2, al    ; Low byte of count (0x200)
    mov al, ch
    out DMA_WORD_COUNT_REG_2, al    ; High byte of count (0x200)
    mov ax, es
    mov cx, 12
    shr ax, cl                      ; Segment to A16-A19
    out DMA_PAGE_ADDR_REG_2, al     ; Page register for upper bits of address
    mov al, 0x02
    out DMA_MASK_REG, al            ; Unmask channel 2
    pop cx
    pop ax
    ret

; Configures a DMA transfer on channel 2 for writing
; Pass a buffer address in ES:DX. ES must be aligned to a 4K boundary.
; Size of transfer should be in CX, and less than 64K
prepare_dma2_write:
    call prepare_dma2

    push ax
    mov al, 0x06
    out DMA_MASK_REG, al            ; Mask channels 0 and 2 during configuration
    mov al, (DMA_MODE_REG_SINGLE | \
                DMA_MODE_REG_READ | \
                DMA_MODE_REG_ADDR_INC | \
                2)                  ; memory -> peripheral ("read")
    out DMA_MODE_REG, al
    mov al, 0x02
    out DMA_MASK_REG, al            ; Unmask channel 2
    pop ax
    ret

; Configures a DMA transfer on channel 2 for reading
; Pass a buffer address in ES:DX. ES must be aligned to a 4K boundary.
; Size of transfer should be in CX, and less than 64K
prepare_dma2_read:
    call prepare_dma2

    push ax
    mov al, 0x06
    out DMA_MASK_REG, al            ; Mask channels 0 and 2 during configuration
    mov al, (DMA_MODE_REG_SINGLE | \
                DMA_MODE_REG_WRITE | \
                DMA_MODE_REG_ADDR_INC | \
                2)                  ; peripheral -> memory ("write")
    out DMA_MODE_REG, al
    mov al, 0x02
    out DMA_MASK_REG, al            ; Unmask channel 2
    pop ax
    ret

; Use a channel 1 memory-to-memory transfer to test the DMA signals
; Should set 1024 bytes at 60:00 to 0xFFFF if successful.
test_dma:
    mov ax, 0x40
    mov ds, ax

    mov ax, 0x60
    mov es, ax
    mov cx, 1024
    xor di, di
    mov ax, 0x55AA
    rep stosw                   ; clear 1K of memory at 60:00

    xor al, al
    out 0x81, al                ; zero out page register

    mov al, 0x01                ; unmask channel 1
    out DMA_MASK_REG, al
    mov al, 0b10000101          ; block DMA transfer, channel 1
    out DMA_MODE_REG, al
    mov al, 0xFF                ; reset flip-flop
    out DMA_CLEAR_FF_REG, al
    mov al, 0x00                ; low byte of address
    out DMA_START_ADDR_REG_1, al
    mov al, 0x06                ; high byte of address
    out DMA_START_ADDR_REG_1, al
    mov al, 0xFF                ; reset flip-flop
    out DMA_CLEAR_FF_REG, al
    mov ax, 1024                ; low byte of count
    out DMA_WORD_COUNT_REG_1, al
    mov al, ah                  ; high byte of count
    out DMA_WORD_COUNT_REG_1, al

    mov al, 0b101               ; set DMA request for channel 1
    out DMA_REQUEST_REG, al

    mov bx, [ds:0x6C]           ; prepare timeout
    add bx, 9                   ; 500ms

    .loop:
    cmp bx, [ds:0x6C]
    jae .timeout
    in al, DMA_STATUS_CMD_REG   ; read status register
    test al, 0x02               ; check for TC on DMA1
    jz .loop

    mov cx, 0
    mov si, 0

    .check:
    cmp word [es:si], 0x55AA
    je .fail
    add si, 2
    cmp si, 1024
    jl .check

    jmp .ok

    .fail:
    mov ax, si
    mov si, dmafail
    call puts

    call puthex
    mov si, crnl
    call puts
    jmp .done

    .timeout:
    mov si, timeout
    call puts
    jmp .done

    .ok:
    mov si, dmaok
    call puts

    .done:
    ret

section .rdata

dmafail db "DMA test failed at ", 0
dmaok db "DMA test passed", 13, 10, 0
crnl db 13, 10, 0
timeout db "DMA test timed out", 13, 10, 0
