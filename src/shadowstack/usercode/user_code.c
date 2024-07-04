#include <stdio.h>
#include "user_code.h"

void user_code()      __attribute__((section(".user_code")));
int sum(int a, int b) __attribute__((section(".user_code")));

int sum(int a, int b) 
{
    int c = a + b;
    return c;
}

void user_code()
{
    int a = 10;
    int b = 20;
    printf("Summing %d and %d\n", a, b);
    int c = sum(a, b);
    printf("Result is %d\n", c);

    // Make the ecall
    asm("ecall");

    asm("j user_mode_exit_point");
}

    // ret = 0;
    // asm volatile("lw %0, 12(sp)" : "=r"(ret));

    // printf("ret in user code after: %08lx\n", ret);