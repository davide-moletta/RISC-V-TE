#include <stdio.h>
#include "intr_vector_table.h"
#include "shadow_stack.h"

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
void esr_handler_instr_addr_mis(void)                                                                  __attribute__((section(".intr_service_routines")));
void esr_handler_instr_acc_fault(void)                                                                 __attribute__((section(".intr_service_routines")));
void esr_handler_illegal_instr(void)                                                                   __attribute__((section(".intr_service_routines")));
void esr_handler_breakpoint(void)                                                                      __attribute__((section(".intr_service_routines")));
void esr_handler_load_addr_mis(void)                                                                   __attribute__((section(".intr_service_routines")));
void esr_handler_load_acc_fault(void)                                                                  __attribute__((section(".intr_service_routines")));
void esr_handler_AMO_addr_mis(void)                                                                    __attribute__((section(".intr_service_routines")));
void esr_handler_AMO_acc_fault(void)                                                                   __attribute__((section(".intr_service_routines")));
void esr_handler_U_mode_ecall(unsigned long ecall_code, unsigned long dst_address, unsigned long mepc) __attribute__((section(".intr_service_routines")));
void esr_handler_S_mode_ecall(void)                                                                    __attribute__((section(".intr_service_routines")));
void esr_handler_M_mode_ecall(void)                                                                    __attribute__((section(".intr_service_routines")));
void esr_handler_instr_page_fault(void)                                                                __attribute__((section(".intr_service_routines")));
void esr_handler_load_page_fault(void)                                                                 __attribute__((section(".intr_service_routines")));
void esr_handler_AMO_page_fault(void)                                                                  __attribute__((section(".intr_service_routines")));
void esr_handler_reserved(void)                                                                        __attribute__((section(".intr_service_routines")));

__attribute__((section(".shadow_stack"))) SStack shadow_stack = {.top = -1};

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


        XLEN-1      30               0
        ------------------------------
        | Interrupt | exception Code |
        ------------------------------
             1             31

        Interrupt is a single bit set to 1 for interrupts and to 0 for exceptions
        exception code contains the code of what triggered the exception/interrupt
    */

    unsigned long mcause, a0, a1;
    unsigned long mepc = 0;
    asm volatile("csrr %0, mcause" : "=r"(mcause)); // Load mcause to inspect the cause of the trap
    asm volatile("add %0, a0, x0" : "=r"(a0));      // Load a0 which contains the ecall code
    asm volatile("add %0, a1, x0" : "=r"(a1));      // Load a1 which contains the additional address

    // Adjust the mepc to point to the next instruction after ecall
    asm("csrr t0, mepc");
    // asm volatile("add %0, t0, x0" : "=r"(mepc));     // Load mepc to retrieve the ecall adress
    asm("addi t0, t0, 4");
    asm("csrw mepc, t0");

    // Check the MSB (bit 31) of the mcause register
    if (mcause & 0x80000000)
    {
        isr_user_software(); // If it is set call the user software interrupt
    }
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
        esr_handler_U_mode_ecall(a0, a1, mepc);
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

    // Return to execution
    asm("mret");
}


/*
    Exception Service Routines functions
*/
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

/*
    CUSTOM ECALL CODES

    Ecall code (a0) | a1 | a2 | Description
            1       | -  | -  | Used to terminate the execution
            2       | -  | -  | Used to check jump instruction
            3       | -  | -  | Used to check return instruction

    To check jump instruction:
        - mepc
        - jump destination
        - CFG entry (if exists)

        - If allowed store return address (jump instruction + 4) in shadow stack

    To check return instruction:
        - return address (ra)
        - mepc
        - shadow stack popped value

        - If allowed do nothing since value already popped

*/
void esr_handler_U_mode_ecall(unsigned long ecall_code, unsigned long dst_address, unsigned long mepc)
{
    // if a0 contains 1 we terminate the execution
    if (ecall_code == 1)
    {
        printf("\t[ESR - U mode ecall]:\tTerminating execution ...\n");
        asm("la t0, terminate_execution");
        asm("csrw mepc, t0");
        asm("mret");
    } else if (ecall_code == 2)
    {
        printf("\t[ESR - U mode ecall]:\tJump check requested for %8lx ...\n", dst_address);
        /*
            CFI CHECK with mepc + 4 and dst address
            definire area di memoria per la stack
            salvare base address stack
            tengo indice per accedere alla posizione corretta
        */

       /*
            IF ALLOWED PUSH mepc + 8 TO STACK asm
       */
        push(&shadow_stack, mepc);

    } else if (ecall_code == 3)
    {
        printf("\t[ESR - U mode ecall]:\tReturn check requested for %8lx ...\n", dst_address);

        /*
            SHADOW STACK CHECK with dst address and popped address
        */
        // pop(&stack);
    }

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

/*
    Interrupt Service Routine functions
*/

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
