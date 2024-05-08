SRCDIR := $(shell pwd)

CC := gcc
CFLAGS := -g3 -ggdb -O3 -Wall -Wextra -Werror -fsanitize=address -MMD
LDFLAGS :=  -fsanitize=address

AS := nasm
ASFLAGS := -f obj -g

C_OBJS := build/linker/main.o build/linker/obj.o build/linker/font.o
C_DEPENDS := ${C_OBJS:.o=.d}

C16 := /mnt/g/Runtimes/digitalmars/bin/dmc.exe
C16_OBJS := build/rom/main.o16 build/rom/string.o16 build/rom/fdc.o16 build/rom/io.o16 build/rom/pit.o16 build/rom/bios.o16 build/rom/ata.o16 build/rom/ints.o16 build/rom/pic.o16 build/rom/kbc.o16 build/rom/serial.o16
C16_FLAGS := -0 -ansi -S
C16_DEPENDS := ${C16_OBJS:.o16=.d}

ROM_OBJS := build/rom/rom.obj build/rom/optionrom.obj build/rom/isr.obj build/rom/kbc.obj build/rom/dma.obj

OBJDUMP := ~/local/bin/objdump
OBJCOPY := ~/local/bin/objcopy

all: mkdirs build/rom.bin build/rom.hex build/rom-opt.bin build/disasm.s

clean:
	@rm -rf build

.PHONY: mkdirs

mkdirs:
	@mkdir -p build/rom build/linker

build/%.obj: %.asm
	$(AS) $(ASFLAGS) -o $@ $<

build/%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

build/romlink: $(C_OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

# Raw ROMBIOS
build/rom.bin: $(ROM_OBJS) $(C16_OBJS) build/romlink
	./build/romlink -o $@ $(ROM_OBJS) $(C16_OBJS) 2>&1 | tee build/romlink.log

# a.out version (for better disassembly)
build/rom.aout: $(ROM_OBJS) $(C16_OBJS) build/romlink
	./build/romlink -a -o $@ $(ROM_OBJS) $(C16_OBJS) 2>&1 | tee build/romlink.log

# Option ROM version (just BIOS interrupts)
build/rom-opt.bin: $(ROM_OBJS) $(C16_OBJS) build/romlink
	./build/romlink -v -t -o $@ $(ROM_OBJS) $(C16_OBJS) 2>&1 | tee build/romlink.log

build/disasm.s: build/rom.aout
	$(OBJDUMP) -D -b a.out-i386 -m i8086 build/rom.aout >build/disasm.s

build/rom.hex: build/rom.bin
	$(OBJCOPY) -D -I binary -B i8086 -O ihex $< $@

build/%.o16: %.c $(build/%.dep)
	$(C16) $(C16_FLAGS) -mtdw -o$@ -d -c -o -0 -NC $<
	python3 deps.py $@ $(shell basename $(<:.c=.dep)) $(@:.o16=.d)

-include ${C_DEPENDS}
-include ${C16_DEPENDS}

