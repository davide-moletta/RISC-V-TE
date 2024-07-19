#include <stdio.h>
#include "user_code.h"
// #include "intr_vector_table.h"
// #include "shadow_stack.h"

void user_code()                 __attribute__((section(".user_code")));
int sum(int a, int b)            __attribute__((section(".user_code")));
int multiplication(int a, int b) __attribute__((section(".user_code")));

int sum(int a, int b)
{
    int sum_result = a + b;

    printf("Result of sum is: %d\nMultiplying %d and %d\n", sum_result, a, b);

    int mul_result = multiplication(a, b);
    printf("Result of mul is %d\n", mul_result);

    // Since sum is not a leaf we must check its return address
    asm("lw a7, 12(sp)");  // Load return address from stack
    asm("addi a7, a7, 1"); // Add 1 to inform that this is a return check
    asm("ecall");          // Perform ecall

    return sum_result;
}

int multiplication(int a, int b)
{
    int mul = a * b;
    return mul;
}

void user_code()
{
    // Temporary to test secureness of PMP
    // printf("Trying to push\n");
    // push(&shadow_stack, 10);
    // printf("pushed %d\n", pop(&shadow_stack));
    
    int first_num = 10;
    int second_num = 20;
    printf("Summing %d and %d\n", first_num, second_num);

    // Since sum is not a leaf we need to store its return address
    asm("li a6, 2");   // Load the number of parameters (not needed when instrumented)
    asm("la a7, sum"); // Load the destination address
    asm("ecall");      // Perform ecall

    sum(first_num, second_num);
}