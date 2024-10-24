#include <stdio.h>
#include "user_code.h"

/*
    BITCOUNT
    
    NORMAL RUN:
        - Binary size: 3584 Byte
        - Execution time: 0.09552812576293945s

    SECURE RUN:
        - Binary size: 4016 Byte
        - Execution time: 0.11546017646789551s
        - Instrumentation time: 0.0019583702087402344s
        - Simulation time: 0s (no indirect jumps)
        - CFG extraction time: 0.0015304088592529297s
*/

int bitcount(unsigned int n) {

    printf("Using %d\n",n);
    if (n == 0)
        return 0;

    return 1 + bitcount(n & (n - 1));
}

void start_u_code()
{
    unsigned int num = 43274821;  // 10100101000101001001000101

    int result = bitcount(num);

    printf("Number of 1-bits in %u: %d\n", num, result); // Expected 10 bits

}
