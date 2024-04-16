global thing
global thing2

segment code public align=16 use16 class=code

thing:
    mov ax, 0xff
    ; first byte in data segment (and dgroup group)
    mov bx, [foo]
    ; first byte in rdata segment (based on dgroup group, which would be implied anyway)
    mov si, bar wrt dgroup
    ; 5th byte in data segment (and dgroup group)
    mov di, [baz]
    ret

thing2:
    mov ax, 0x55AA
    ret

segment data public align=4 use16 class=data

foo:
    dd 0xdeadbeef
baz:
    db 0x42

segment rdata public align=4 use16 class=data

bar:
    dd 0x1337cafe

group dgroup data rdata
