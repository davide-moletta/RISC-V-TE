#include <stdio.h>
#include "user_entry.h"
#include "user_code.h"

void user_mode_entry_point() __attribute__((section(".user_code")));
void user_mode_exit_point()  __attribute__((section(".user_code")));

void user_mode_entry_point()
{
    printf("\n\n--- Start of user code ---\n\n");

    // Call function here
    user_code();
    // asm("ecall");

}

void user_mode_exit_point()
{
    printf("\n\n--- End of user code ---\n\n");

    // Ecall to terminate execution
    asm("li a0, 1");
    asm("ecall");
}