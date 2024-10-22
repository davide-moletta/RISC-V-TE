#ifndef SHADOW_STACK_H
#define SHADOW_STACK_H

short push(unsigned int address) __attribute__((section(".intr_service_routines")));
unsigned int pop()               __attribute__((section(".intr_service_routines")));

#endif
