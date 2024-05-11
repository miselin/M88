bits 16
cpu 8086

segment _TEXT public align=16 use16 class=CODE

global call_option_roms
global call_video_bios

call_video_bios:
    ; look for a BIOS at 0xC000
    mov ax, 0xC000
    mov es, ax
    xor di, di

    cmp word [es:di], 0xAA55
    jne .no_bios

    ; BIOS is here, call it
    call 0xC000:0x0003

    .no_bios:
    ret

call_option_roms:
    ; load option ROMs if present
    ; they start at 0xC8000 (after VIDEO BIOS) and end at 0xF0000
    mov ax, 0xC800
    mov es, ax
    mov di, 0x0000

    .optionroms:
    mov es, ax
    cmp ax, 0xF000
    jge .optionroms_done

    xor di, di
    mov dx, [es:di]
    cmp dx, 0xAA55
    jne .next_optionrom

    ; third byte is size of ROM in 512-byte blocks
    ; use that to skip directly to next ROM instead of counting in 512 byte increments
    xor dx, dx
    mov dl, byte [es:2]
    ; (512 * dx) >> 4
    mov cx, 5
    sal dx, cl

    .run_optionrom:

    push ax ; save current segment
    push dx ; save size of current ROM
    mov bx, 0x50
    mov ds, bx

    ; off:seg of option ROM entry point for far call
    mov word [ds:7], ax
    mov word [ds:5], 3
    call far [ds:5]
    pop dx
    pop ax

    add ax, dx
    mov es, ax

    jmp .optionroms

    .next_optionrom:
    add ax, 0x20 ; 512 bytes (in segment, so it will be left shifted 4 bits)
    mov es, ax
    jmp .optionroms

    .optionroms_done:
    ret
