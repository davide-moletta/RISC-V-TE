#include <stdio.h>
#include "user_code.h"

/*
    NORMAL RUN:
        - Binary size: 10560 Byte
        - Execution time: 3.254150390625s

    SECURE RUN:
        - Binary size: 10768 Byte
        - Execution time: 3.254135847091675s
        - Instrumentation time: 0.0021398067474365234s
        - Simulation time: 0s (no indirect jumps)
        - CFG extraction time: 0.0016832351684570312s
*/

int isPrime(int i, int num)
{
    printf("Checking: %d - %d\n", i, num);
    if (num == i)
    {
        return 0;
    }
    else if (num % i == 0)
    {
        return 1;
    }

    return isPrime(i + 1, num);
}

void start_u_code()
{
    int target_one = 1637;
    int target_two = 7000;

    if (isPrime(2, target_one) == 1)
    {
        printf("%d is a prime number.\n", target_one);
    }
    else
    {
        printf("%d is not a prime number.\n", target_one);
    }

    if (isPrime(2, target_two) == 1)
    {
        printf("%d is a prime number.\n", target_two);
    }
    else
    {
        printf("%d is not a prime number.\n", target_two);
    }
}
