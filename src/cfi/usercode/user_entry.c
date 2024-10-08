#include <stdio.h>
#include "user_entry.h"
#include "user_code.h"

void user_mode_entry_point()
{
    printf("\n\n--- Start of user code ---\n\n");

    user_code(); // Call first user function here

    printf("\n\n--- End of user code ---\n\n");

    asm("li a7, 1"); // Load 1 in a0 (code to terminate execution)
    asm("ecall");    // Ecall to terminate execution
}