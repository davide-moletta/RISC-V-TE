#ifndef INTR_VECTOR_TABLE_H
#define INTR_VECTOR_TABLE_H

#include "shadow_stack.h"

extern SStack shadow_stack;

// Interrupt vector table with all the calls to interrupt service routines
void interrupt_vector_table(void) __attribute__((section(".interrupt_vector_table")));

#endif