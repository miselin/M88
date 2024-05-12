bits 16
cpu 8086

segment _TEXT public align=16 use16 class=CODE

global toupper

; AL = character to convert to uppercase (returned in AL)
toupper:
    cmp al, 'a'
    jb .done
    cmp al, 'z'
    ja .done
    sub al, 32
    .done:
    ret
