#include <stdio.h>
#include "user_code.h"

void user_code()                 __attribute__((section(".user_code")));
int sum(int a, int b)            __attribute__((section(".user_code")));
int multiplication(int a, int b) __attribute__((section(".user_code")));

int sum(int a, int b) 
{
    // Save context
    asm("add sp, sp, -16");
    asm("sw ra, 12(sp)");

    int sum = a + b;

    /*
        CODE INSTRUMENTATION
    */
    asm("lw t0, 12(sp)"); // Load return address from stack
    asm("mv a1, t0");     // Load the destination address in a1
    asm("li a0, 3");      // Load the code for the return check
    asm("ecall");         // Perform ecall
    
    // Restore context
    asm("lw ra, 12(sp)");
    asm("add sp, sp, 16");

    return sum;
}

int multiplication(int a, int b) 
{
    // Save context
    asm("add sp, sp, -16");
    asm("sw ra, 12(sp)");

    int mul = a * b;

    /*
        CODE INSTRUMENTATION
    */
    asm("lw t0, 12(sp)"); // Load return address from stack
    asm("mv a1, t0");     // Load the destination address in a1
    asm("li a0, 3");      // Load the code for the return check
    asm("ecall");         // Perform ecall
    
    // Restore context
    asm("lw ra, 12(sp)");
    asm("add sp, sp, 16");

    return mul;
}

void user_code()
{
    int a = 10;
    int b = 2;
    printf("Summing %d and %d\n", a, b);

    /*
        CODE INSTRUMENTATION
    */
    asm("la a1, sum");  // Load the destination address
    asm("li a0, 2");    // Load the code for the jump check
    asm("ecall");       // Perform ecall

    int c = sum(a, b);
    printf("Result of sum is %d\n", c);

    asm("la a1, multiplication");  // Load the destination address
    asm("li a0, 2");    // Load the code for the jump check
    asm("ecall");       // Perform ecall

    int d = multiplication(a, b);
    printf("Result of mul is %d\n", d);

    asm("j user_mode_exit_point");
}