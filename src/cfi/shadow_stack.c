#include <stdio.h>

#define MAX_SIZE 63 

typedef struct {
    unsigned int addresses[MAX_SIZE];
    int top;
} SStack;

short isFull()                   __attribute__((section(".intr_service_routines")));
short isEmpty()                  __attribute__((section(".intr_service_routines")));
short push(unsigned int address) __attribute__((section(".intr_service_routines")));
unsigned int pop()               __attribute__((section(".intr_service_routines")));

__attribute__((section(".shadow_stack"))) SStack shadow_stack = {.top = -1};

// Check if the stack is full
short isFull() {
    return shadow_stack.top == MAX_SIZE - 1;
}

// Check if the stack is empty
short isEmpty() {
    return shadow_stack.top == -1;
}

// Push an address into the stack
short push(unsigned int address) {
    if (isFull()) {
        return 0; // Stack is full
    }
    shadow_stack.addresses[++shadow_stack.top] = address;
    return 1;
}

// Pop an address from the stack
unsigned int pop() {
    if (isEmpty()) {
        return 0; // Stack is empty, return 0
    }
    return shadow_stack.addresses[shadow_stack.top--];
}
