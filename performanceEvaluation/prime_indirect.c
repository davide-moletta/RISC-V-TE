#include <stdio.h>
#include "user_code.h"

/*
    IS PRIME INDIRECT
    
    NORMAL RUN:
        - Binary size: 3712 Byte
        - Execution time: 3.2537381649017334s

    SECURE RUN:
        - Binary size: 10768 Byte
        - Execution time: 3.254897117614746s
        - Instrumentation time: 0.0020825862884521484s
        - Simulation time: 11.405126333236694s
        - CFG extraction time: 0.0045146942138671875s
*/

int isPrime(int i, int num)
{
    int (*volatile funPtr)(int, int);
    funPtr = isPrime;
    printf("Checking: %d - %d\n", i, num);
    if (num == i)
    {
        return 0;
    }
    else if (num % i == 0)
    {
        return 1;
    }

    return funPtr(i + 1, num);
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
