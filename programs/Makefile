SRCDIR := $(shell pwd)

CC := gcc
CFLAGS := -g3 -ggdb -O3 -Wall -Wextra -Werror -fsanitize=address -MMD
LDFLAGS := -fsanitize=address

AS := nasm
ASFLAGS := -f elf -g

ROMLINK_OBJS := build/linker/main.o build/linker/obj.o build/linker/font.o
ROMLINK_DEPENDS := ${ROMLINK_OBJS:.o=.d}

ELF2BIN_OBJS := build/elf2bin/main.o
ELF2BIN_DEPENDS := ${ELF2BIN_OBJS:.o=.d}

ROM_OBJS := build/rom/rom.obj build/rom/optionrom.obj build/rom/isr.obj build/rom/kbc.obj build/rom/dma.obj build/rom/pic.obj
ROM_OBJS += build/rom/pit.obj build/rom/bios.obj build/rom/fdc.obj build/rom/strio.obj build/rom/tables.obj build/rom/mem.obj
ROM_OBJS += build/rom/string.obj build/rom/serial.obj build/rom/parallel.obj build/rom/logo.obj

LD := ld
OBJDUMP := objdump
OBJCOPY := objcopy

all: mkdirs build/rom.bin build/rom.hex build/rom-opt.bin build/disasm.s build/opt-disasm.s

clean:
	@rm -rf build

.PHONY: mkdirs

mkdirs:
	@mkdir -p build/rom build/linker build/elf2bin

build/%.obj: %.asm
	$(AS) $(ASFLAGS) -o $@ $<

build/%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

build/romlink: $(ROMLINK_OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

build/elf2bin/elf2bin: $(ELF2BIN_OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

# ELF version of ROMBIOS
build/rom.elf: $(ROM_OBJS) rom/link.ld
	./stamps.sh
	$(AS) $(ASFLAGS) -o build/stamp.obj build/stamp.asm
	$(LD) -Trom/link.ld -o $@ $(ROM_OBJS) build/stamp.obj

# Raw ROMBIOS
build/rom.bin: build/rom.elf build/elf2bin/elf2bin
	./build/elf2bin/elf2bin build/rom.elf $@

# Option ROM version (just BIOS interrupts) - includes requisite checksum
build/rom-opt.bin: build/rom.elf build/romlink
	./build/elf2bin/elf2bin -c build/rom.elf $@

# Main ROM disassembly
build/disasm.s: build/rom.elf
	$(OBJDUMP) -S -m i8086 build/rom.elf >build/disasm.s

# Option ROM disassembly
build/opt-disasm.s: build/rom-opt.bin
	$(OBJDUMP) -D -b binary -m i8086 build/rom-opt.bin >build/opt-disasm.s

# Programming file to write to EEPROM
build/rom.hex: build/rom.bin
	$(OBJCOPY) -D -I binary -B i8086 -O ihex $< $@

-include ${ROMLINK_DEPENDS}
-include ${ELF2BIN_DEPENDS}
