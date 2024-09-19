# The M88 Computer - ROMs / Programs

Run `make` to build.

## `elf2bin`

`elf2bin` converts a linked ELF binary to a ROM image, placing each segment
at its associated memory location in the binary file.

It completely ignores relocations.

## `rom`

The `rom` directory contains the main ROM code. This can be used as the main
BIOS ROM for this repository's computer schematic. It can also be loaded as
an Option ROM to simplify testing ROM code in PC emulators like Bochs.

## linker

The `linker` directory contains a buggy OMF linker that was used before
`elf2bin`.
