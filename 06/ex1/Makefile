INCLUDEDIR=./include
CFLAGS=-ansi -Wpedantic -Wall -Werror
all: main.exe

main.exe: main.o dimezza.o quadrato.o
	gcc ${CFLAGS} -o main.exe main.o dimezza.o quadrato.o -lm

main.o: main.c ${INCLUDEDIR}/dimezza.h ${INCLUDEDIR}/quadrato.h
	gcc -c ${CFLAGS} -I${INCLUDEDIR} main.c

dimezza.o: dimezza.c
	gcc -c ${CFLAGS} dimezza.c

quadrato.o: quadrato.c
	gcc -c ${CFLAGS} quadrato.c

.PHONY: clean

clean:
	rm -f main.exe *.o *~ core