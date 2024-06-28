#include <stdio.h>
#include <usercode/user_code.h>

void user_mode_entry_point() __attribute__((section(".user_code")));

void user_mode_entry_point()
{
    printf("\n\n--- Start of user code ---\n\n");

    // call function here
    user_code();
    
    printf("\n\n--- End of user code ---\n\n");
}