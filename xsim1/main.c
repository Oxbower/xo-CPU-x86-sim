#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "xis.h"
#include "xcpu.h"
#include "xmem.h"

int main(int argc, char **argv) {
    xcpu * INSTRUCT = calloc(1, sizeof(xcpu)); //instantiate xcpu
    xmem_init(XIS_MEM_SIZE);                          //instantiate memory
    unsigned int MAX_CYCLE = atoi(argv[1]);           //ascii to integer
    unsigned int INFINITE_LOOP = 1;                        //check if program is set to run a specific amount
    if (MAX_CYCLE != 0)
        INFINITE_LOOP = 0;
    FILE *f = fopen(argv[2], "rb");

    unsigned char buffer[2];    //memory write buffer
    unsigned int addr = 0;      //address to write at

    while (!feof(f))
    {
        fread(buffer, 2, 1, f);
        xmem_store(&buffer[0], addr);         //store the byte in memory
        xmem_store(&buffer[1], addr + 1);//store the byte in memory
        addr += 2;                                 //move memory pointer 'forwards' by a word
    }

    unsigned int CURRENT_CYCLE = 0;

    while (1)
    {
        if (!INFINITE_LOOP) { //cycle counter
            if (CURRENT_CYCLE >= MAX_CYCLE)
                break;
            CURRENT_CYCLE++;
        }
        if (xcpu_execute(INSTRUCT) == 0)
            break;
    }
  return 0;
}