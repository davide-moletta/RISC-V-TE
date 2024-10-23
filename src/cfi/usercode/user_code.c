#include <stdio.h>
#include <stdlib.h>
#include "user_code.h"

int division(int a, int b) { return a / b; }

int multiplication(int a, int b)
{
    int mul_result = a * b;
    printf("Result of mul is %d\n", mul_result);
    return mul_result;
}

int sum(int a, int b)
{
    int sum_result = a + b;

    printf("Result of sum is: %d\nMultiplying %d and %d\n", sum_result, a, b);

    multiplication(a, b);

    return sum_result;
}

int diff(int a, int b)
{
    int diff_result = a - b;

    printf("Result of diff is: %d\nDividing %d and %d\n", diff_result, a, b);

    printf("Result of div is %d\n", division(a, b));

    return diff_result;
}

void start_u_code()
{
    int first_num = 20, second_num = 10;
    int (*volatile funPtr)(int, int);

    printf("Summing %d and %d\n", first_num, second_num);
    funPtr = sum;
    funPtr(first_num, second_num);

    printf("Subtracting %d and %d\n", first_num, second_num);
    diff(first_num, second_num);
}
