#include <stdio.h>

void print_reg(unsigned int src, unsigned int dst) __attribute__((section(".ij_logger")));

// Function used in simulation to print source-destination pairs for indirect jumps
void print_reg(unsigned int src, unsigned int dst){
    printf("Source: %x - Destination: %x\n", src, dst);
}
