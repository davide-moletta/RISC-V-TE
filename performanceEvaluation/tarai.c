#include <stdio.h>
#include "user_code.h"

/*
    TARAI  
    
    NORMAL RUN:
        - Binary size: 3616 Byte
        - Execution time: 0.09367251396179199s

    SECURE RUN:
        - Binary size: 4128 Byte
        - Execution time: 0.09395313262939453s
        - Instrumentation time: 0.001976490020751953s
        - Simulation time: 0s (no indirect jumps)
        - CFG extraction time: 0.0015201568603515625s
*/

int tarai(int x, int y, int z)
{
    printf("x: %d, y: %d, z: %d\n", x, y, z);
    if (x == 0)
    {
        return y;
    }
    else if (y == 0)
    {
        return tarai(x - 1, y + 1, z);
    }
    else if (z == 0)
    {
        return tarai(x, y - 1, z + 1);
    }
    else
    {
        return tarai(x - 1, y + 1, z);
    }
}

void start_u_code()
{
    int x = 7, y = 10, z = 12;
    int result = tarai(x, y, z);
    printf("Tarai(%d, %d, %d) = %d\n", x, y, z, result);
}
