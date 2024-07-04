#ifndef SHADOW_STACK_H
#define SHADOW_STACK_H

#define MAX_SIZE 100

typedef struct
{
    unsigned long addresses[MAX_SIZE];
    int top;
} SStack;

int push(SStack* stack, unsigned long address) __attribute__((section(".shadow_stack")));
unsigned long pop(SStack* stack)               __attribute__((section(".shadow_stack")));

#endif
