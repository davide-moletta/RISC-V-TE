#include <stdio.h>
#include "user_code.h"

void user_code()                 __attribute__((section(".user_code")));
int sum(int a, int b)            __attribute__((section(".user_code")));
int multiplication(int a, int b) __attribute__((section(".user_code")));

int sum(int a, int b) 
{   
    int sum = a + b;
    
    /*
        CODE INSTRUMENTATION
    */
    asm("lw t0, 12(sp)"); // Load return address from stack
    asm("mv a1, t0");     // Load the destination address in a1
    asm("li a0, 3");      // Load the code for the return check
    asm("ecall");         // Perform ecall
    
    return sum;
}

int multiplication(int a, int b) 
{
    int mul = a * b;

    /*
        CODE INSTRUMENTATION
    */
    asm("lw t0, 12(sp)"); // Load return address from stack
    asm("mv a1, t0");     // Load the destination address in a1
    asm("li a0, 3");      // Load the code for the return check
    asm("ecall");         // Perform ecall
    
    return mul;
}

void user_code()
{
    int a = 10;
    int b = 20;
    printf("Summing %d and %d\n", a, b);

    /*
        CODE INSTRUMENTATION
    */
    asm("la a1, sum"); // Load the destination address
    asm("li a0, 2");   // Load the code for the jump check
    asm("ecall");      // Perform ecall

    int c = sum(a, b);
    printf("Result of sum is %d\n", c);

    printf("Multipying %d and %d\n", a, b);
    asm("la a1, multiplication");  // Load the destination address
    asm("li a0, 2");               // Load the code for the jump check
    asm("ecall");                  // Perform ecall

    int d = multiplication(a, b);
    printf("Result of mul is %d\n", d);

    asm("j user_mode_exit_point");
}