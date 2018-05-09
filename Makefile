CC = gcc
CFLAGS =

all: emu

emu: cpu.c main.c
		$(CC) $(CFLAGS) -o emu cpu.c main.c -lpthread
test: cpu.c test.c
		$(CC) $(CFLAGS) -o test cpu.c test.c
clean:
		$(RM) emu
