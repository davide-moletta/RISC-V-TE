#ifndef SHADOW_STACK_H
#define SHADOW_STACK_H

#define MAX_SIZE 63

typedef struct
{
    unsigned int addresses[MAX_SIZE];
    short top;
} SStack;

short push(SStack *stack, unsigned int address) __attribute__((section(".intr_service_routines")));
unsigned int pop(SStack *stack)                 __attribute__((section(".intr_service_routines")));
void initStack(SStack *stack)                   __attribute__((section(".intr_service_routines")));

#endif
