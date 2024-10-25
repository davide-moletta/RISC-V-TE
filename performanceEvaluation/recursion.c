#include <stdio.h>
#include "user_code.h"

/*
    INEFFICIENT RECURSION

    NORMAL RUN:
        - Binary size: 3568 Byte
        - Execution time:  5.537966251373291s

    SECURE RUN:
        - Binary size: 19808 Byte
        - Execution time: 5.539685653686523s
        - Instrumentation time: 0.0020227432250976562s
        - Simulation time: 0s (no indirect jumps)
        - CFG extraction time: 0.0015854835510253906s
*/

int recursion(int n)
{
    printf("Checking: %d\n", n);
    if (n == 1)
    {
        return n;
    }
    return recursion(n - 1);
}

void start_u_code()
{
    int n = 3999;
    recursion(n);
    printf("Reached 1 starting from %d\n", n);
}
