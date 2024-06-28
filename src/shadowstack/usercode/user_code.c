#include <stdio.h>

void user_code()      __attribute__((section(".user_code")));
int sum(int a, int b) __attribute__((section(".user_code")));

void user_code()
{
    int a = 10;
    int b = 20;
    printf("Summing %d and %d\n", a, b);
    int c = sum(a, b);
    printf("Result is %d\n", c);

    asm("ecall");

    printf("CODE after ecall\n");
}

int sum(int a, int b) 
{
    int c = a + b;
    return c;
}

    // unsigned long ret;
    // asm volatile ("add %0, ra, x0" : "=r" (ret));

	// printf("ret: %ld\n", ret);
