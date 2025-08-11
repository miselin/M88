export WATCOM = /opt/watcom
export INCLUDE = $(WATCOM)/h
export PATH := $(WATCOM)/binl:$(PATH)

export RELEASE_DIR = /tmp/release-i2c

CC = $(WATCOM)/binl/wcl

all: i2c.lib i2c.exe i2c.asm

lib.o: lib.c i2c.h
	$(CC) -c -zastd=c99 -bt=dos -0 $<

i2c.lib: lib.o
	wlib i2c.lib -+lib.o

i2c.exe: i2c.c i2c.lib
	$(CC) -zastd=c99 -bc -bt=dos -0 -fe=$@ i2c.c i2c.lib

i2c.asm: i2c.exe
	$(WATCOM)/binl/wdis -l=$@ -a i2c.o

clean:
	rm -f *.obj *.sys

release.zip: README.txt i2c.exe i2c.c i2c.h lib.c i2c.lib
	rm --preserve-root -rf $(RELEASE_DIR) && mkdir -p $(RELEASE_DIR)/i2c
	cp $^ $(RELEASE_DIR)/i2c/
	cd $(RELEASE_DIR); zip $(PWD)/$@ i2c/*
