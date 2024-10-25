#include <stdio.h>
#include "user_code.h"

/*
    FIBONACCI INDIRECT

    NORMAL RUN:
        - Binary size: 3600 Byte
        - Execution time: 0.08495879173278809s

    SECURE RUN:
        - Binary size: 4064 Byte
        - Execution time: 0.15105891227722168s
        - Instrumentation time: 0.0020172595977783203s
        - Simulation time: 130.5566909313202s
        - CFG extraction time: 0.07036638259887695s
*/

int fibonacci(int n)
{
    int (*volatile funPtr)(int);
    funPtr = fibonacci;
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
        return (funPtr(n - 1) + funPtr(n - 2));
    }
}

void start_u_code()
{
    int n = 20;

    printf("Fibbonacci of %d: ", n);

    for (int i = 0; i < n; i++)
    {
        printf("%d ", fibonacci(i));
    }
}
