#include <stdio.h>
#include <user_code.h>

void user_mode_entry_point(void)           __attribute__((section(".user_code")));

void user_mode_entry_point(void)
{
    // asm("la t0, 0x0");
    // asm("csrw mie, t0");

    // asm("add a0, a0, 4");
    // asm("ecall");
    
    // asm("j main + 1");

    printf("Hello from user mode!\n");

    // call function here
    user_code();
    
    asm("ret");
}