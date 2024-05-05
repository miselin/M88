G:\Runtimes\digitalmars\bin\dmc.exe -mtdw -d -c -o -0 rom\main.c -obuild\rom\main.o16 -lbuild\rom\main.lst
G:\Runtimes\digitalmars\bin\dmc.exe -mtdw -c -d -o -0 rom\string.c -obuild\rom\string.o16 -lbuild\rom\string.lst
G:\Runtimes\digitalmars\bin\dmc.exe -mtdw -c -d -o -0 rom\fdc.c -obuild\rom\fdc.o16 -lbuild\rom\fdc.lst
G:\Runtimes\digitalmars\bin\dmc.exe -mtdw -c -d -o -0 rom\io.c -obuild\rom\io.o16 -lbuild\rom\io.lst
G:\Runtimes\digitalmars\bin\dmc.exe -mtdw -c -d -o -0 rom\pit.c -obuild\rom\pit.o16 -lbuild\rom\pit.lst

wsl.exe -e /usr/bin/make
