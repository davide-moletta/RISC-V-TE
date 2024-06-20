#include <stdio.h>

void interrupt_vector_table(void)           __attribute__((section(".intr_vector_table")));

void synchronous_exception_handler(void)    __attribute__((section(".intr_service_routines")));
void isr_supervisor_software(void)          __attribute__((section(".intr_service_routines")));
void isr_reserved(void)                     __attribute__((section(".intr_service_routines")));
void isr_machine_software(void)             __attribute__((section(".intr_service_routines")));
void isr_user_timer(void)                   __attribute__((section(".intr_service_routines")));
void isr_supervisor_timer(void)             __attribute__((section(".intr_service_routines")));
void isr_machine_timer(void)                __attribute__((section(".intr_service_routines")));
void isr_user_external(void)                __attribute__((section(".intr_service_routines")));
void isr_supervisor_external(void)          __attribute__((section(".intr_service_routines")));
void isr_machine_external(void)             __attribute__((section(".intr_service_routines")));

/*
Interrupt | Exception Code | Description
1         | 0              | User software interrupt
1         | 1              | Supervisor software interrupt
1         | 2              | Reserved
1         | 3              | Machine software interrupt
1         | 4              | User timer interrupt
1         | 5              | Supervisor timer interrupt
1         | 6              | Reserved
1         | 7              | Machine timer interrupt
1         | 8              | User external interrupt
1         | 9              | Supervisor external interrupt
1         | 10             | Reserved
1         | 11             | Machine external interrupt
1         | ≥12            | Reserved
------------------------------------------------------------
0         | 0              | Instruction address misaligned
0         | 1              | Instruction access fault
0         | 2              | Illegal instruction
0         | 3              | Breakpoint
0         | 4              | Load address misaligned
0         | 5              | Load access fault
0         | 6              | Store/AMO address misaligned
0         | 7              | Store/AMO access fault
0         | 8              | Environment call from U-mode
0         | 9              | Environment call from S-mode
0         | 10             | Reserved
0         | 11             | Environment call from M-mode
0         | 12             | Instruction page fault
0         | 13             | Load page fault
0         | 14             | Reserved
0         | 15             | Store/AMO page fault
0         | ≥16            | Reserved
*/

void interrupt_vector_table(void)
{
    __asm__ volatile("j synchronous_exception_handler");
    __asm__ volatile("j isr_supervisor_software");
    __asm__ volatile("j isr_reserved");
    __asm__ volatile("j isr_machine_software");
    __asm__ volatile("j isr_user_timer");
    __asm__ volatile("j isr_supervisor_timer");
    __asm__ volatile("j isr_reserved");
    __asm__ volatile("j isr_machine_timer");
    __asm__ volatile("j isr_user_external");
    __asm__ volatile("j isr_supervisor_external");
    __asm__ volatile("j isr_reserved");
    __asm__ volatile("j isr_machine_external");
    __asm__ volatile("j isr_reserved");
}

void synchronous_exception_handler(void)
{
    printf("Synchronous exception or user software error generated\n");

}

void isr_supervisor_software(void)
{
    printf("Supervisor software interrupt generated");
}
void isr_machine_software(void)
{
    printf("Machine software interrupt generated\n");
}
void isr_user_timer(void)
{
    printf("User timer interrupt generated\n");
}
void isr_supervisor_timer(void)
{
    printf("Supervisor timer interrupt generated");
}
void isr_machine_timer(void)
{
    printf("Machine timer interrupt generated\n");
}
void isr_user_external(void)
{
    printf("User external interrupt generated");
}
void isr_supervisor_external(void)
{
    printf("Supervisor external interrupt generated");
}
void isr_machine_external(void)
{
    printf("Machine external interrupt generated");
}

/*
    Should never be called since reserved
*/
void isr_reserved(void)
{
    printf("\n\n Error: reserved interrupt code used \n\n");
}
