.PHONY: all build clean run

AS = nasm
CC = gcc
LD = ld

all: main.o fat.o error.o command.o my_print.o
	@${CC} -o main $?
	@-rm -f *.o

my_print.o: my_print.asm
	${AS} -f elf64 -o $@ $<

error.o: error.c headers/error.h headers/includes.h
	${CC} -c -o $@ $<

fat.o: fat.c headers/fat.h headers/includes.h headers/command.h
	${CC} -c -o $@ $<

command.o: command.c headers/command.h headers/includes.h
	${CC} -c -o $@ $<

main.o:	main.c headers/fat.h headers/error.h headers/command.h headers/includes.h
	${CC} -c -o $@ $<

main.run: main.o fat.o error.o command.o my_print.o
	@${CC} -o $@ $?
	@./$@

pre.i: main.c fat.c error.c command.c headers/fat.h headers/error.h headers/command.h headers/includes.h
	@${CC} -o $@ -E $<

build: main.run clean

run: main.run clean

clean: 
	@-rm -f *.o *.run
