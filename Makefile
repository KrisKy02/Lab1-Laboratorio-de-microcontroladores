FAMILY=-mpic14
MCU=-p12f683
CC=sdcc
LD=sdcc
CFLAGS=-I. -I/usr/local/share/sdcc/non-free/include
TARGET=dados 

SRCS = dados.c  

all:
	${CC} --use-non-free ${FAMILY} ${MCU} ${CFLAGS} -o ${TARGET} ${SRCS}

clean:
	rm -f *.c~ *.h~ *.o *.elf *.hex *.asm
