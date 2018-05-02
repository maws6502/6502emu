CC = gcc
CFLAGS =

all: emu

emu: emu.c main.c
		$(CC) $(CFLAGS) -o emu emu.c main.c -lpthread
clean:
		$(RM) emu
