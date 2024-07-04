#include "shadow_stack.h"

int isFull(SStack *stack)                      __attribute__((section(".shadow_stack")));
int isEmpty(SStack *stack)                     __attribute__((section(".shadow_stack")));
int push(SStack *stack, unsigned long address) __attribute__((section(".shadow_stack")));
unsigned long pop(SStack *stack)               __attribute__((section(".shadow_stack")));

// Check if the stack is full
int isFull(SStack *stack)
{
    return stack->top == MAX_SIZE - 1;
}

// Check if the stack is empty
int isEmpty(SStack *stack)
{
    return stack->top == -1;
}

// Push an address onto the stack
int push(SStack *stack, unsigned long address)
{
    if (isFull(stack))
    {
        return 0;
    }
    stack->addresses[++stack->top] = address;
    return 1;
}

// Pop an address from the stack
unsigned long pop(SStack *stack)
{
    if (isEmpty(stack))
    {
        return 0;
    }
    return stack->addresses[stack->top--];
}
