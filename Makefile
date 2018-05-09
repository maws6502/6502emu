CC = gcc
CFLAGS =

all: emu test

emu: cpu.c main.c cpu_intrn.h
		$(CC) $(CFLAGS) -o emu cpu.c main.c -lpthread
test: cpu.c test.c cpu_intrn.h
		$(CC) $(CFLAGS) -o test cpu.c test.c
clean:
		$(RM) emu test
