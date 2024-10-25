#include <stdio.h>
#include "user_code.h"

/*
    INEFFICIENT RECURSION INDIRECT

    NORMAL RUN:
        - Binary size: 3584 Byte
        - Execution time: 5.537818670272827s

    SECURE RUN:
        - Binary size: 19808 Byte
        - Execution time: 5.5399863719940186s
        - Instrumentation time: 0.0020236968994140625s
        - Simulation time: 23.27507257461548s
        - CFG extraction time: 0.008749008178710938s
*/

int recursion(int n)
{
    int (*volatile funPtr)(int);
    funPtr = recursion;
    printf("Checking: %d\n", n);
    if (n == 1)
    {
        return n;
    }
    return funPtr(n - 1);
}

void start_u_code()
{
    int n = 3999;
    recursion(n);
    printf("Reached 1 starting from %d\n", n);
}
