CC = gcc
CFLAGS =

all: emu

emu: cpu.c main.c
		$(CC) $(CFLAGS) -o emu cpu.c main.c -lpthread
clean:
		$(RM) emu
