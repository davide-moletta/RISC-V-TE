#include <stdio.h>
#include "user_entry.h"
#include "merge.h"

void user_mode_entry_point() __attribute__((section(".user_code")));

void user_mode_entry_point()
{
    printf("\n\n--- Start of user code ---\n\n");

    mergeSetup(); // Call first user function here

    printf("\n\n--- End of user code ---\n\n");

    asm("li a7, 1"); // Load 1 in a0 (code to terminate execution)
    asm("ecall");    // Ecall to terminate execution
}