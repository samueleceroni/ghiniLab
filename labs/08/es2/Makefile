CFLAGS=-ansi -Wpedantic -Wall -Werror
all: main.exe

main.exe: main.o
	gcc ${CFLAGS} -o main.exe main.o -lpthread

main.o: main.c
	gcc ${CFLAGS} -c main.c

.PHONY: clean

clean:
	-rm -f main.exe *.o *~ core