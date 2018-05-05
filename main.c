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
                if(dev->memory[0xFF0F]) {
                        printf("%c", (char) dev->memory[0xFF0E]);
                        fflush(stdout);
                        dev->memory[0xFF0F] = 0x00;
                }
                if(inchar && !dev->memory[0xFF0D]) {
                        dev->memory[0xFF0C] = inchar;
                        dev->memory[0xFF0D] = 1;
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
        read(fd, dev->memory + 0xC000, 0x4000);
        close(fd);
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
