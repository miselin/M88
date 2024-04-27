CC := gcc
CFLAGS := -g3 -ggdb -O3 -Wall -Wextra -Werror -fsanitize=address -MMD
LDFLAGS :=  -fsanitize=address

AS := nasm
ASFLAGS := -f obj -g

C_OBJS := build/linker/main.o build/linker/obj.o
C_DEPENDS := ${C_OBJS:.o=.d}

C16 := bcc
C16_OBJS := build/rom/main.o16 build/rom/vsprintf.o16 build/rom/string.o16
C16_FLAGS := -0 -ansi -S

ROM_OBJS := build/rom/rom.obj build/rom/optionrom.obj build/rom/isr.obj build/rom/pic.obj build/rom/pit.obj build/rom/kbc.obj build/rom/dma.obj build/rom/fdc.obj build/rom/strio.obj

OBJDUMP := ~/local/bin/objdump
OBJCOPY := ~/local/bin/objcopy

all: mkdirs build/rom.bin build/rom.ith

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

build/rom.bin: $(ROM_OBJS) $(C16_OBJS) build/romlink
	./build/romlink $(ROM_OBJS) $(C16_OBJS) $@ 2>&1 | tee build/romlink.log
	$(OBJDUMP) -D -b a.out-i386 -m i8086 rom.aout >build/disasm.s

build/rom.ith: build/rom.bin
	$(OBJCOPY) -D -I binary -B i8086 -O ihex $< $@

-include ${C_DEPENDS}

