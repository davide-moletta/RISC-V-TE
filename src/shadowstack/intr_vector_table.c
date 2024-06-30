#include <stdio.h>

// Interrupt vector table with all the calls to interrupt service routines
void interrupt_vector_table(void)        __attribute__((section(".intr_vector_table")));

// Interrupt Service Routines to handle interrupts
void synchronous_exception_handler(void) __attribute__((section(".intr_service_routines")));
void isr_user_software(void)             __attribute__((section(".intr_service_routines")));
void isr_supervisor_software(void)       __attribute__((section(".intr_service_routines")));
void isr_reserved(void)                  __attribute__((section(".intr_service_routines")));
void isr_machine_software(void)          __attribute__((section(".intr_service_routines")));
void isr_user_timer(void)                __attribute__((section(".intr_service_routines")));
void isr_supervisor_timer(void)          __attribute__((section(".intr_service_routines")));
void isr_machine_timer(void)             __attribute__((section(".intr_service_routines")));
void isr_user_external(void)             __attribute__((section(".intr_service_routines")));
void isr_supervisor_external(void)       __attribute__((section(".intr_service_routines")));
void isr_machine_external(void)          __attribute__((section(".intr_service_routines")));

// Exception Service Routines to handle exceptions
void esr_handler_instr_addr_mis(void)    __attribute__((section(".intr_service_routines")));
void esr_handler_instr_acc_fault(void)   __attribute__((section(".intr_service_routines")));
void esr_handler_illegal_instr(void)     __attribute__((section(".intr_service_routines")));
void esr_handler_breakpoint(void)        __attribute__((section(".intr_service_routines")));
void esr_handler_load_addr_mis(void)     __attribute__((section(".intr_service_routines")));
void esr_handler_load_acc_fault(void)    __attribute__((section(".intr_service_routines")));
void esr_handler_AMO_addr_mis(void)      __attribute__((section(".intr_service_routines")));
void esr_handler_AMO_acc_fault(void)     __attribute__((section(".intr_service_routines")));
void esr_handler_U_mode_ecall(void)      __attribute__((section(".intr_service_routines")));
void esr_handler_S_mode_ecall(void)      __attribute__((section(".intr_service_routines")));
void esr_handler_M_mode_ecall(void)      __attribute__((section(".intr_service_routines")));
void esr_handler_instr_page_fault(void)  __attribute__((section(".intr_service_routines")));
void esr_handler_load_page_fault(void)   __attribute__((section(".intr_service_routines")));
void esr_handler_AMO_page_fault(void)    __attribute__((section(".intr_service_routines")));
void esr_handler_reserved(void)          __attribute__((section(".intr_service_routines")));

/*
Interrupt | exception Code | Description
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
    /*
        MCAUSE CSR Shown as

        bit(s) position
        ---------------
        | bit(s) name |
        ---------------
        bit(s) length


        31          30               0
        ------------------------------
        | Interrupt | exception Code |
        ------------------------------
             1             31

        Interrupt is a single bit set to 1 for interrupts and to 0 for exceptions
        exception code contains the code of what triggered the exception/interrupt
    */

    // Load mcause to inspect the cause of the trap
    unsigned long mcause;
    asm volatile("csrr %0, mcause" : "=r"(mcause));

    // Check the MSB (bit 31) of the mcause register
    if (mcause & 0x80000000)
    {
        isr_user_software(); // If it is set call the user software interrupt
    }
    else
    {
        // If not then it's an exception so check the cause
        if ((mcause & 0xFF) == 0) // Check if it's an Instruction address misaligned
        {
            esr_handler_instr_addr_mis();
        }
        else if ((mcause & 0xFF) == 1) // Check if it's an Instruction access fault
        {
            esr_handler_instr_acc_fault();
        }
        else if ((mcause & 0xFF) == 2) // Check if it's an Illegal instruction
        {
            esr_handler_illegal_instr();
        }
        else if ((mcause & 0xFF) == 3) // Check if it's a Breakpoint
        {
            esr_handler_breakpoint();
        }
        else if ((mcause & 0xFF) == 4) // Check if it's a Load address misaligned
        {
            esr_handler_load_addr_mis();
        }
        else if ((mcause & 0xFF) == 5) // Check if it's a Load access fault
        {
            esr_handler_load_acc_fault();
        }
        else if ((mcause & 0xFF) == 6) // Check if it's a Store/AMO address misaligned
        {
            esr_handler_AMO_addr_mis();
        }
        else if ((mcause & 0xFF) == 7) // Check if it's a Store/AMO access fault
        {
            esr_handler_AMO_acc_fault();
        }
        else if ((mcause & 0xFF) == 8) // Check if it's an Environment call from U-mode
        {
            unsigned long a0;
            asm volatile("add %0, a0, x0" : "=r"(a0));

            // if a0 contains 1 we terminate the execution
            if (a0 == 1)
            {
                asm("la t0, terminate_execution");
                asm("csrw mepc, t0");
                asm("mret");
            }
            esr_handler_U_mode_ecall();
        }
        else if ((mcause & 0xFF) == 9) // Check if it's an Environment call from S-mode
        {
            esr_handler_S_mode_ecall();
        }
        else if ((mcause & 0xFF) == 11) // Check if it's an Environment call from M-mode
        {
            esr_handler_M_mode_ecall();
        }
        else if ((mcause & 0xFF) == 12) // Check if it's an Instruction page fault
        {
            esr_handler_instr_page_fault();
        }
        else if ((mcause & 0xFF) == 13) // Check if it's a Load page fault
        {
            esr_handler_load_page_fault();
        }
        else if ((mcause & 0xFF) == 15) // Check if it's a Store/AMO page fault
        {
            esr_handler_AMO_page_fault();
        }
        else if (((mcause & 0xFF) == 10) || ((mcause & 0xFF) == 14) || ((mcause & 0xFF) >= 16)) // If it's 10, 14 or >= 16 it's reserved
        {
            esr_handler_reserved();
        }
    }

    // Adjust the mepc to point to the next instruction after ecall
    asm("csrr t0, mepc");
    asm("addi t0, t0, 4");
    asm("csrw mepc, t0");

    // Return to execution
    asm("mret");
}

void esr_handler_instr_addr_mis(void)
{
    printf("Handling exception Instruction Address Misaligned \n");
}
void esr_handler_instr_acc_fault(void)
{
    printf("Handling exception Instruction Access Fault \n");
}
void esr_handler_illegal_instr(void)
{
    printf("Handling exception Illegal Instruction \n");
}
void esr_handler_breakpoint(void)
{
    printf("Handling exception Breakpoint \n");
}
void esr_handler_load_addr_mis(void)
{
    printf("Handling exception Load Address Misaligned \n");
}
void esr_handler_load_acc_fault(void)
{
    printf("Handling exception Load Access Fault \n");
}
void esr_handler_AMO_addr_mis(void)
{
    printf("Handling exception Store/AMO Address Misaligned \n");
}
void esr_handler_AMO_acc_fault(void)
{
    printf("Handling exception Store/AMO Access Fault \n");
}
void esr_handler_U_mode_ecall(void)
{
    printf("Handling exception U-mode Ecall \n");

}
void esr_handler_S_mode_ecall(void)
{
    printf("Handling exception S-mode Ecall \n");
}
void esr_handler_M_mode_ecall(void)
{
    printf("Handling exception M-mode Ecall \n");
}
void esr_handler_instr_page_fault(void)
{
    printf("Handling exception Intruction Page Fault \n");
}
void esr_handler_load_page_fault(void)
{
    printf("Handling exception Load Page Fault \n");
}
void esr_handler_AMO_page_fault(void)
{
    printf("Handling exception Store/AMO Page Fault \n");
}
/*
    Should never be called since reserved
*/
void esr_handler_reserved(void)
{
    printf("\n\n Error: reserved exception code used \n\n");
}

void isr_user_software(void)
{
    printf("User software interrupt generated");
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
