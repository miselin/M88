export WATCOM = /opt/watcom
export INCLUDE = $(WATCOM)/h
export PATH = $(WATCOM)/binl:$PATH

CC = $(WATCOM)/binl/wcl

all: i2c.exe

i2c.exe: i2c.c
	$(CC) -zastd=c99 -bc -bt=dos -0 -fe=$@ $<

clean:
	rm -f *.obj *.sys
