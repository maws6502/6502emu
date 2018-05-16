#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "cpu.h"

int
main(int argc, char *argv[])
{
        struct stat st;
        int fd, i;
        Emu65Device *dev;
        if (argc != 2){
                printf("Need ROM\n");
                return 1;
        }
        dev = malloc(sizeof(Emu65Device));
        fd = open(argv[1], O_RDONLY);
        read(fd, dev->memory, 0x10000);
        close(fd);
        emu65_reset(dev);
        dev->pc = 0x0400;
        while(!emu65_cycle(dev)) {
                printf("PC=%x AC=%x X=%x Y=%x SR=%x SP=%x\n", dev->pc, dev->ac, dev->x, dev->y, dev->sr, dev->sp); 
        }

        return 0;
}
