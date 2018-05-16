#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdint.h>
#include <fcntl.h>
#include <pthread.h>
#include <termios.h>
#include "cpu.h"

char inchar = 0;

struct basicio {
        uint8_t mem[0x10000];
        uint8_t in;
};

uint8_t
memread(struct basicio *io, uint16_t addr)
{
        uint8_t *mem = io->mem;
        uint8_t val;
        if (addr == 0xF004) {
                val = mem[addr];
                mem[addr] = 0x00;
                return val;
        }
        return mem[addr];
}

void
memwrite(struct basicio *io, uint16_t addr, uint8_t val)
{
        uint8_t *mem = io->mem;
        mem[addr] = val;
        if (addr == 0xF001) {
                printf("%c", mem[addr]);
                fflush(stdout);
        }
        return;
}

void *
runCPU(void *i)
{
        Emu65Device *dev;
        dev = (Emu65Device *) i;
        for (;;) {
                uint16_t i;
                if (i = emu65_cycle(dev)) {
                        printf("INVALID INSTRUCTION. HALTING AT %x\n", i);
                        break;
                }
                if(inchar) {
                        dev->memwrite(dev->param, 0xF004, inchar);
                        inchar = 0;
                }
        }
        return 0;
}

int
main(int argc, char *argv[])
{
        struct stat st;
        int fd, i;
        pthread_t cputhread;
        Emu65Device *dev;
        struct basicio *io;
        if (argc != 2){
                printf("Need ROM\n");
                return 1;
        }
        stat(argv[1], &st);
        if(st.st_size != 16384){
                printf("ROM must be exactly 16kb\n");
                return 1;
        }
        dev = malloc(sizeof(Emu65Device));
        io = malloc(sizeof(struct basicio));
        fd = open(argv[1], O_RDONLY);
        read(fd, io->mem + 0xC000, 0x4000);
        close(fd);
        dev->memread = memread;
        dev->memwrite = memwrite;
        dev->param = io;
        emu65_reset(dev);
        pthread_create(&cputhread, NULL, runCPU, (void *) dev);

        struct termios trm;
        tcgetattr(STDIN_FILENO, &trm);
        trm.c_lflag &= ~(ECHO | ICANON);
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &trm);

        char c;
        while (read(STDIN_FILENO, &c, 1) == 1) {
                if (c == 0x0a) c = 0x0d;
                inchar = c;
        }

        return 0;
}
