CFLAGS=-ansi -Wpedantic -Wall -Werror
MACRO=-D'ALLOCAT(x)=do{int i; x = (int*) malloc (10*sizeof(int));if(x!=NULL){for(i=0;i<10;i++){x[i] = i-1000;}}} while (0)'

all: main.exe

main.exe: main.o
	gcc ${CFLAGS} -o main.exe main.o

main.o: main.c
	gcc -c ${CFLAGS} ${MACRO} main.c

.PHONY: clean

clean:
	-rm main.exe main.o *~ core
