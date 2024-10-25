#include <stdio.h>
#include "user_code.h"

/*
    FIBONACCI RECURSIVE

    NORMAL RUN:
        - Binary size: 3584 Byte
        - Execution time: 0.616631269454956s

    SECURE RUN:
        - Binary size: 4608 Byte
        - Execution time: 10.00898003578186s
        - Instrumentation time: 0.0020914077758789062s
        - Simulation time: 0s (no indirect jumps)
        - CFG extraction time: 0.0014908313751220703s
*/

int fibbonacci(int n)
{
    if (n == 0)
    {
        return 0;
    }
    else if (n == 1)
    {
        return 1;
    }
    else
    {
        return (fibbonacci(n - 1) + fibbonacci(n - 2));
    }
}

void start_u_code()
{
    int n = 30;

    printf("Fibbonacci of %d: ", n);

    for (int i = 0; i < n; i++)
    {
        printf("%d ", fibbonacci(i));
    }
}
