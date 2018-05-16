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

uint8_t
memread(uint8_t *mem, uint16_t addr)
{
        return mem[addr];
}

void
memwrite(uint8_t *mem, uint16_t addr, uint8_t val)
{
        mem[addr] = val;
        return;
}

void *
runCPU(void *i)
{
        Emu65Device *dev;
        dev = (Emu65Device *) i;
        for(;;) {
                uint16_t i;
                if (i = emu65_cycle(dev)) {
                        printf("INVALID INSTRUCTION. HALTING AT %x\n", i);
                        break;
                }
                if(dev->memread(dev->param, 0x800F)) {
                        printf("%c", (char) dev->memread(dev->param, 0x800E));
                        fflush(stdout);
                        dev->memwrite(dev->param, 0x800F, 0x00);
                }
                if(inchar && !dev->memread(dev->param, 0xFF0D)) {
                        dev->memwrite(dev->param, 0x800C, inchar);
                        dev->memwrite(dev->param, 0x800D, 1);
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
        uint8_t mem[0x10000];
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
        fd = open(argv[1], O_RDONLY);
        read(fd, mem + 0xC000, 0x4000);
        close(fd);
        dev->memread = memread;
        dev->memwrite = memwrite;
        dev->param = mem;
        emu65_reset(dev);
        pthread_create(&cputhread, NULL, runCPU, (void *) dev);

        struct termios trm;
        tcgetattr(STDIN_FILENO, &trm);
        trm.c_lflag &= ~(ECHO | ICANON);
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &trm);

        char c;
        while (read(STDIN_FILENO, &c, 1) == 1) {
                inchar = c;
        }

        return 0;
}
