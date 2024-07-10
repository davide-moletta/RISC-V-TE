#include <stdio.h>
#include "user_code.h"

void user_code()                 __attribute__((section(".user_code")));
int sum(int a, int b)            __attribute__((section(".user_code")));
int multiplication(int a, int b) __attribute__((section(".user_code")));

int sum(int a, int b)
{
    int sum_result = a + b;

    printf("Result of sum is: %d\nMultipying %d and %d\n", sum_result, a, b);

    int mul_result = multiplication(a, b);
    printf("Result of mul is %d\n", mul_result);

    return sum_result;
}

int multiplication(int a, int b)
{
    int mul = a * b;
    return mul;
}

void user_code()
{
    int first_num = 10;
    int second_num = 20;
    printf("Summing %d and %d\n", first_num, second_num);

    sum(first_num, second_num);
}

/*
    CODE INSTRUMENTATION

    For jump checks
    asm("la a1, sum"); // Load the destination address
    asm("li a0, 2");   // Load the code for the jump check
    asm("ecall");      // Perform ecall

    For return checks
    asm("lw t0, 12(sp)"); // Load return address from stack
    asm("mv a1, t0");     // Load the destination address in a1
    asm("li a0, 3");      // Load the code for the return check
    asm("ecall");         // Perform ecall
*/