CFLAGS =-ansi -Wpedantic -Wall -Werror -D_THREAD_SAFE -D_REENTRANT -D_POSIX_C_SOURCE=200112L
all: main.exe

main.exe: main.o DBGpthread.o
	gcc ${CFLAGS} -o main.exe main.o DBGpthread.o -lpthread

main.o: main.c
	gcc ${CFLAGS} -c main.c
DBGpthread.o:	DBGpthread.c printerror.h
	gcc ${CFLAGS} -c DBGpthread.c -I./

.PHONY: clean

clean:
	-rm -f main.exe *.o *~ core