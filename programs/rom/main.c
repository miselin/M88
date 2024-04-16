int rommain() {
  asm {
        mov ax, 0x01
        out 0xF0, al
  }

  return 0;
}