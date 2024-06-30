#include <stdio.h>
// #include <user_code.h>

void user_mode_entry_point() __attribute__((section(".user_code")));
int sum(int a, int b) __attribute__((section(".user_code")));

int sum(int a, int b)
{
    int c = a + b;
    return c;
}

void user_mode_entry_point()
{
    printf("\n\n--- Start of user code ---\n\n");

    // call function here
    // user_code();

    int a = 10;
    int b = 20;
    printf("Summing %d and %d\n", a, b);
    int c = sum(a, b);
    printf("Result is %d\n", c);

    // unsigned long rett;
    // asm volatile("add %0, ra, x0" : "=r"(rett));

    // printf("ret in user code after sum: %08lx\n", rett);

    asm("ecall");

    printf("CODE after ecall\n");

    // unsigned long rettt;
    // asm volatile("add %0, ra, x0" : "=r"(rettt));

    // printf("ret in user code after ecall: %08lx\n", rettt);

    printf("\n\n--- End of user code ---\n\n");
    asm("li a0, 1");
    asm("ecall");
}