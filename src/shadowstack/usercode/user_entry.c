#include <stdio.h>
#include <usercode/user_code.h>

void user_mode_entry_point(void) __attribute__((section(".user_code")));

void user_mode_entry_point(void)
{
    printf("\n\n--- Start of user code ---\n");

    // call function here
    user_code();
    
    printf("--- End of user code ---\n\n");

    asm("ret");
}