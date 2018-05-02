#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdint.h>
#include <fcntl.h>
#include <pthread.h>
#include <termios.h>
#include "emu.h"

char inchar = 0;

void *
runCPU()
{
        for(;;) {
                if (cycle()) {
                        printf("INVALID INSTRUCTION. HALTING\n");
                        break;
                }
                if(memory[0xFF0F]) {
                        printf("%c", (char) memory[0xFF0E]);
                        fflush(stdout);
                        memory[0xFF0F] = 0x00;
                }
                if(inchar && !memory[0xFF0D]) {
                        memory[0xFF0C] = inchar;
                        memory[0xFF0D] = 1;
                        inchar = 0;
                }
        }
        return;
}

int
main(int argc, char *argv[])
{
        struct stat st;
        int fd, i;
        pthread_t cputhread;
        if (argc != 2){
                printf("Need ROM\n");
                return 1;
        }
        stat(argv[1], &st);
        if(st.st_size != 16384){
                printf("ROM must be exactly 16kb\n");
                return 1;
        }
        fd = open(argv[1], O_RDONLY);
        read(fd, memory + 0xC000, 0x4000);
        close(fd);
        reset_cpu();
        pthread_create(&cputhread, NULL, runCPU, (void *) 0);

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
