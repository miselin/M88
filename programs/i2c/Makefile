export WATCOM = /opt/watcom
export INCLUDE = $(WATCOM)/h
export PATH = $(WATCOM)/binl:$PATH

CC = $(WATCOM)/binl/wcl

all: i2c.lib i2c.exe

lib.o: lib.c i2c.h
	$(CC) -c -zastd=c99 -bt=dos -0 $<

i2c.lib: lib.o
	wlib i2c.lib -+lib.o

i2c.exe: i2c.c i2c.lib
	$(CC) -zastd=c99 -bc -bt=dos -0 -fe=$@ i2c.c i2c.lib

clean:
	rm -f *.obj *.sys
