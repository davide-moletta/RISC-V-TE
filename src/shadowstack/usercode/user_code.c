#include <stdio.h>

void user_code(void)  __attribute__((section(".user_code")));
int sum(int a, int b) __attribute__((section(".user_code")));

void user_code(void)
{
    int a = 10;
    int b = 20;
    printf("Summing %d and %d\n", a, b);
    int c = sum(a, b);
    printf("Result is %d\n", c);
}

int sum(int a, int b) 
{
    int c = a + b;
    return c;
}