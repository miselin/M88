bits 16
cpu 8086

segment code public align=16 use16 class=code

global _call_option_roms

_call_option_roms:
; save all registers, as option ROMs may clobber them
push ax
push bx
push cx
push dx
push es
push ds
push di
push si

; load option ROMs if present, including VGABIOS which typically sits at 0xC0000
; they start at 0xC0000 and end at 0xF0000
mov ax, 0xC000
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

pop si
pop di
pop ds
pop es
pop dx
pop cx
pop bx
pop ax
ret

segment data public align=4 use16 class=data

segment rdata public align=4 use16 class=data

group dgroup data rdata