.PHONY: build clean run

AS = nasm
CC = gcc
LD = ld

my_print.o: my_print.asm
	${AS} -f elf64 -o $@ $<

error.o: error.c headers/error.h headers/includes.h
	${CC} -c -o $@ $<

fat.o: fat.c headers/fat.h headers/includes.h
	${CC} -c -o $@ $<

command.o: command.c headers/command.h headers/includes.h
	${CC} -c -o $@ $<

main.o:	main.c headers/fat.h headers/error.h headers/command.h headers/includes.h
	${CC} -c -o $@ $<

fat12Driver: main.o my_print.o
	# ${LD} -o $@ $? -lc --dynamic-linker /lib64/ld-linux-x86-64.so.2
	${LD} -o $@ $?

build: fat12Driver clean

main.run: main.o fat.o error.o command.o
	@${CC} -o $@ $?
	@./$@

run: main.run clean

clean: 
	@-rm -f *.o *.run
