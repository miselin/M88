bits 16
cpu 8086

segment _DATA public align=16 use16 class=DATA

global _kbascii
global _kbascii_shift
global _kbascii_ctrl
global _kbascii_alt
global kbnumpad
global kbnumpad_numlock

_kbascii:
    db 0x00     ; 0x00
    db 0x1B     ; ESC
    db '1'
    db '2'
    db '3'
    db '4'
    db '5'
    db '6'
    db '7'
    db '8'
    db '9'
    db '0'
    db '-'
    db '='
    db 0x08     ; Backspace
    db 0x09     ; Tab
    db 'q'      ; 0x10
    db 'w'
    db 'e'
    db 'r'
    db 't'
    db 'y'
    db 'u'
    db 'i'
    db 'o'
    db 'p'
    db '['
    db ']'
    db 0x0D     ; Enter
    db 0x00 ; LCtrl
    db 'a'
    db 's'
    db 'd'      ; 0x20
    db 'f'
    db 'g'
    db 'h'
    db 'j'
    db 'k'
    db 'l'
    db ';'
    db 0x27 ; '
    db '`'
    db 0x00 ; LShift
    db 0x5C     ; Backslash
    db 'z'
    db 'x'
    db 'c'
    db 'v'
    db 'b'      ; 0x30
    db 'n'
    db 'm'
    db ','
    db '.'
    db '/'
    db 0x00  ; RShift
    db '*'
    db 0x00  ; LAlt
    db ' '
    db 0x00  ; CapsLock
    db 0x00  ; F1
    db 0x00  ; F2
    db 0x00  ; F3
    db 0x00  ; F4
    db 0x00  ; F5
    db 0x00  ; 0x40 - F6
    db 0x00  ; F7
    db 0x00  ; F8
    db 0x00  ; F9
    db 0x00  ; F10
    db 0x00  ; NumLock
    db 0x00  ; ScrollLock

_kbascii_shift:
    db 0x00
    db 0x1B  ; ESC
    db '!'
    db '@'
    db '#'
    db '$'
    db '%'
    db '^'
    db '&'
    db '*'
    db '('
    db ')'
    db '_'
    db '+'
    db 0x08     ; Backspace
    db 0x00
    db 'Q'
    db 'W'
    db 'E'
    db 'R'
    db 'T'
    db 'Y'
    db 'U'
    db 'I'
    db 'O'
    db 'P'
    db '{'
    db '}'
    db 0x0D     ; Enter
    db 0x00  ; LCtrl
    db 'A'
    db 'S'
    db 'D'
    db 'F'
    db 'G'
    db 'H'
    db 'J'
    db 'K'
    db 'L'
    db ':'
    db '"'
    db '~'
    db 0x00  ; LShift
    db '|'
    db 'Z'
    db 'X'
    db 'C'
    db 'V'
    db 'B'
    db 'N'
    db 'M'
    db '<'
    db '>'
    db '?'
    db 0x00  ; RShift
    db 0xFF
    db 0x00  ; LAlt
    db ' '
    db 0x00  ; CapsLock
    db 0x00  ; F1
    db 0x00  ; F2
    db 0x00  ; F3
    db 0x00  ; F4
    db 0x00  ; F5
    db 0x00  ; F6
    db 0x00  ; F7
    db 0x00  ; F8
    db 0x00  ; F9
    db 0x00  ; F10
    db 0x00  ; NumLock
    db 0x00  ; ScrollLock

_kbascii_ctrl:
    db 0x00
    db 0x1B  ; ESC
    db 0xFF
    db 0x00
    db 0xFF
    db 0xFF
    db 0xFF
    db 0x1E
    db 0xFF
    db 0xFF
    db 0xFF
    db 0xFF
    db 0x1F
    db 0xFF
    db 0x7F
    db 0x00
    db 'q' - 0x60
    db 'w' - 0x60
    db 'e' - 0x60
    db 'r' - 0x60
    db 't' - 0x60
    db 'y' - 0x60
    db 'u' - 0x60
    db 'i' - 0x60
    db 'o' - 0x60
    db 'p' - 0x60
    db 0x1B
    db 0x1D
    db 0x0A     ; Enter
    db 0x00  ; LCtrl
    db 'a' - 0x60
    db 's' - 0x60
    db 'd' - 0x60
    db 'f' - 0x60
    db 'g' - 0x60
    db 'h' - 0x60
    db 'j' - 0x60
    db 'k' - 0x60
    db 'l' - 0x60
    db 0xFF
    db 0xFF
    db 0xFF
    db 0x00  ; LShift
    db 0x1C
    db 'z' - 0x60
    db 'x' - 0x60
    db 'c' - 0x60
    db 'v' - 0x60
    db 'b' - 0x60
    db 'n' - 0x60
    db 'm' - 0x60
    db 0xFF
    db 0xFF
    db 0xFF
    db 0x00  ; RShift
    db 0x00
    db 0x00  ; LAlt
    db ' '
    db 0x00  ; CapsLock
    db 0x00  ; F1
    db 0x00  ; F2
    db 0x00  ; F3
    db 0x00  ; F4
    db 0x00  ; F5
    db 0x00  ; F6
    db 0x00  ; F7
    db 0x00  ; F8
    db 0x00  ; F9
    db 0x00  ; F10
    db 0x00  ; NumLock
    db 0x00  ; ScrollLock

_kbascii_alt:
    db 0x00
    db 0x1B  ; ESC
    db 0x00
    db 0x00
    db 0x00
    db 0x00
    db 0x00
    db 0x00
    db 0x00
    db 0x00
    db 0x00
    db 0x00
    db 0x00
    db 0x00
    db 0x00
    db 0x00
    db 0x00
    db 0x00
    db 0x00
    db 0x00
    db 0x00
    db 0x00
    db 0x00
    db 0x00
    db 0x00
    db 0x00
    db 0x00
    db 0x00
    db 0x00
    db 0x00  ; LCtrl
    db 0x00
    db 0x00
    db 0x00
    db 0x00
    db 0x00
    db 0x00
    db 0x00
    db 0x00
    db 0x00
    db 0x00
    db 0xFF
    db 0xFF
    db 0x00  ; LShift
    db 0x00
    db 0x00
    db 0x00
    db 0x00
    db 0x00
    db 0x00
    db 0x00
    db 0x00
    db 0xFF
    db 0xFF
    db 0xFF
    db 0x00  ; RShift
    db 0x00
    db 0x00  ; LAlt
    db ' '
    db 0x00  ; CapsLock
    db 0x00  ; F1
    db 0x00  ; F2
    db 0x00  ; F3
    db 0x00  ; F4
    db 0x00  ; F5
    db 0x00  ; F6
    db 0x00  ; F7
    db 0x00  ; F8
    db 0x00  ; F9
    db 0x00  ; F10
    db 0x00  ; NumLock
    db 0x00  ; ScrollLock

kbnumpad:
    db 0x00     ; NP 7
    db 0x00     ; NP 8
    db 0x00     ; NP 9
    db '-'      ; NP -
    db 0x00     ; NP 4
    db 0xFF     ; NP 5
    db 0x00     ; NP 6
    db '+'      ; NP +
    db 0x00     ; NP 1
    db 0x00     ; NP 2
    db 0x00     ; NP 3
    db 0x00     ; NP 0
    db 0x00     ; NP .

kbnumpad_numlock:
    db '7'      ; NP 7
    db '8'      ; NP 8
    db '9'      ; NP 9
    db '-'      ; NP -
    db '4'      ; NP 4
    db '5'      ; NP 5
    db '6'      ; NP 6
    db '+'      ; NP +
    db '1'      ; NP 1
    db '2'      ; NP 2
    db '3'      ; NP 3
    db '0'      ; NP 0
    db '.'      ; NP .
