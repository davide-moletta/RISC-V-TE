#include <stdio.h>

void user_code(void)           __attribute__((section(".user_code")));

void user_code(void)
{
    printf("Hello from user function!\n");
}