#include <stdio.h>
#include <time.h>
#include <stdlib.h> 
#include "user_code.h"
// #include "intr_vector_table.h"
// #include "shadow_stack.h"

void user_code()                 __attribute__((section(".user_code")));
int sum(int a, int b)            __attribute__((section(".user_code")));
int multiplication(int a, int b) __attribute__((section(".user_code")));
int diff(int a, int b)           __attribute__((section(".user_code")));
int division(int a, int b)       __attribute__((section(".user_code")));

int diff(int a, int b)
{
    int diff_result = a - b;

    printf("Result of diff is: %d\nDividing %d and %d\n", diff_result, a, b);

    int div_result = division(a, b);
    printf("Result of div is %d\n", div_result);

    return diff_result;
}

int division(int a, int b)
{
    int div = a / b;
    return div;
}

int sum(int a, int b)
{
    int sum_result = a + b;

    printf("Result of sum is: %d\nMultiplying %d and %d\n", sum_result, a, b);

    int mul_result = multiplication(a, b);
    printf("Result of mul is %d\n", mul_result);

    // Since sum is not a leaf we must check its return address
    // asm("lw a7, 12(sp)");  // Load return address from stack
    // asm("addi a7, a7, 1"); // Add 1 to inform that this is a return check
    // asm("ecall");          // Perform ecall

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
    
    // srand((unsigned int)(time(NULL)));
    // int random = rand();  

    int first_num = 20;
    int second_num = 10;

    if(first_num > second_num)
    {
        printf("Summing %d and %d\n", first_num, second_num);
        // Since sum is not a leaf we need to store its return address
        // asm("la a7, sum"); // Load the destination address
        // asm("ecall");      // Perform ecall
        // asm("li a6, 2");   // Load the number of parameters (not needed when instrumented)
        sum(first_num, second_num);
    } else
    {
        printf("Subtracting %d and %d\n", first_num, second_num);
        // Since diff is not a leaf we need to store its return address
        // asm("la a7, sum"); // Load the destination address
        // asm("ecall");      // Perform ecall
        // asm("li a6, 2");   // Load the number of parameters (not needed when instrumented)
        diff(first_num, second_num);
    } 
}