.PHONY: build clean

AS = nasm
CC = gcc
LD = ld

my_print.o: my_print.asm
	${AS} -f elf64 -o $@ $<

main.o:	main.c
	${CC} -c -o $@ $<

fat12Driver: main.o my_print.o
	# ${LD} -o $@ $? -lc --dynamic-linker /lib64/ld-linux-x86-64.so.2
	${LD} -o $@ $?

build: fat12Driver clean

clean: 
	-rm -f *.o
