G:\Runtimes\digitalmars\bin\dmc.exe -msdw -c -d -o -0 rom\main.c -obuild\rom\main.o16 -lbuild\rom\main.lst
G:\Runtimes\digitalmars\bin\dmc.exe -msdw -c -d -o -0 rom\vsprintf.c -obuild\rom\vsprintf.o16 -lbuild\rom\vsprintf.lst
G:\Runtimes\digitalmars\bin\dmc.exe -msdw -c -d -o -0 rom\string.c -obuild\rom\string.o16 -lbuild\rom\string.lst

wsl.exe -e /usr/bin/make
