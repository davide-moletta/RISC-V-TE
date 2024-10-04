#include "shadow_stack.h"

short isFull(SStack *stack)                     __attribute__((section(".intr_service_routines")));
short isEmpty(SStack *stack)                    __attribute__((section(".intr_service_routines")));
short push(SStack *stack, unsigned int address) __attribute__((section(".intr_service_routines")));
unsigned int pop(SStack *stack)                 __attribute__((section(".intr_service_routines")));
void initStack(SStack *stack)                   __attribute__((section(".intr_service_routines")));

// Check if the stack is full
short isFull(SStack *stack)
{
    return stack->top == MAX_SIZE - 1;
}

// Check if the stack is empty
short isEmpty(SStack *stack)
{
    return stack->top == -1;
}

// Push an address onto the stack
short push(SStack *stack, unsigned int address)
{
    if (isFull(stack))
    {
        return 0;
    }
    stack->addresses[++stack->top] = address;
    return 1;
}

// Pop an address from the stack
unsigned int pop(SStack *stack)
{
    if (isEmpty(stack))
    {
        return 0;
    }
    return stack->addresses[stack->top--];
}

void initStack(SStack *stack)
{
    for (int i = 0; i < MAX_SIZE; i++)
    {
        stack->addresses[i] = 1;
    }
}