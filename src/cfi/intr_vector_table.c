#include <stdio.h>
#include "intr_vector_table.h"
#include "shadow_stack.h"
#include "cfg.h"

// Interrupt vector table with all the calls to interrupt service routines
void interrupt_vector_table(void) __attribute__((section(".interrupt_vector_table")));

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
void esr_handler_U_mode_ecall(unsigned int ecode_address_encoding, unsigned int mepc) __attribute__((section(".intr_service_routines")));
void esr_handler_S_mode_ecall(void)      __attribute__((section(".intr_service_routines")));
void esr_handler_M_mode_ecall(unsigned int ecode_address_encoding, unsigned int mepc) __attribute__((section(".intr_service_routines")));
void esr_handler_instr_page_fault(void)  __attribute__((section(".intr_service_routines")));
void esr_handler_load_page_fault(void)   __attribute__((section(".intr_service_routines")));
void esr_handler_AMO_page_fault(void)    __attribute__((section(".intr_service_routines")));
void esr_handler_reserved(void)          __attribute__((section(".intr_service_routines")));
 
void code_termination(void)              __attribute__((section(".intr_service_routines")));

__attribute__((section(".shadow_stack"))) SStack shadow_stack = {.top = -1};
__attribute__((section(".cfg"))) CFG cfg = {.sources = {1077415716, 1077415840, 1077416014}, .destinations = {1077415942, 1077415758, 1077415868}};

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
    asm volatile("j synchronous_exception_handler");
    asm volatile("j isr_supervisor_software");
    asm volatile("j isr_reserved");
    asm volatile("j isr_machine_software");
    asm volatile("j isr_user_timer");
    asm volatile("j isr_supervisor_timer");
    asm volatile("j isr_reserved");
    asm volatile("j isr_machine_timer");
    asm volatile("j isr_user_external");
    asm volatile("j isr_supervisor_external");
    asm volatile("j isr_reserved");
    asm volatile("j isr_machine_external");
    asm volatile("j isr_reserved");
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

    asm volatile("csrc mstatus, %0" :: "r"(1 << 0)); // Disable user-level interrupts by clearing the UIE bit in mstatus CSR

    unsigned int mcause, mepc, a7;
    asm volatile("csrr %0, mcause" : "=r"(mcause)); // Load mcause to inspect the cause of the trap
    asm volatile("csrr %0, mepc" : "=r"(mepc));     // Load mepc to retrieve the ecall address
    asm volatile("add %0, a7, x0" : "=r"(a7));      // Load a7 which contains the addition address and ecall encoded code

    // Check the MSB (bit 31) of the mcause register
    if (mcause & 0x80000000)
    {
        isr_user_software(); // If it is set call the user software interrupt
    }   // If not then it's an exception so check the cause
    else if ((mcause & 0xFF) == 0) // Check if it's an Instruction address misaligned
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
        esr_handler_U_mode_ecall(a7, mepc);
    }
    else if ((mcause & 0xFF) == 9) // Check if it's an Environment call from S-mode
    {
        esr_handler_S_mode_ecall();
    }
    else if ((mcause & 0xFF) == 11) // Check if it's an Environment call from M-mode
    {
        esr_handler_M_mode_ecall(a7, mepc);
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
    else // If it's 10, 14 or >= 16 it's reserved
    {
        esr_handler_reserved();
    }

    // Reset mstatus to U mode before turning back
    asm("csrr t0, mstatus");                         // Load MSTATUS in t0
    asm("li t1, 0xFFFFE7FF");                        // Load user mode status in t1
    asm("and t0, t0, t1");                           // And to change MPP bits in MSTATUS to user mode
    asm("or t0, t0, 8");                             // Or to change MIE bits in MSTATUS to 1
    asm("csrw mstatus, t0");                         // Write new MSTATUS
    asm volatile("csrs mstatus, %0" :: "r"(1 << 0)); // Enable user-level interrupts by setting the UIE bit in mstatus CSR

    // Adjust the mepc to point to the next instruction after ecall
    asm("csrr t0, mepc");
    asm("addi t0, t0, 4");
    asm("csrw mepc, t0");

    // Return to execution
    asm("mret");
}

/*
    Exception Service Routines functions
*/
void esr_handler_instr_addr_mis(void)
{
    printf("\t[ESR - Instruction Address Misaligned]:\n");
}
void esr_handler_instr_acc_fault(void)
{
    printf("\t[ESR - Instruction Access Fault]: \n");
}
void esr_handler_illegal_instr(void)
{
    printf("\t[ESR - Illegal Instruction]: \n");
}
void esr_handler_breakpoint(void)
{
    printf("\t[ESR - Breakpoint]: \n");
}
void esr_handler_load_addr_mis(void)
{
    printf("\t[ESR - Load Address Misaligned]: \n");
}
void esr_handler_load_acc_fault(void)
{
    printf("\t[ESR - Load Access Fault]: \n");
}
void esr_handler_AMO_addr_mis(void)
{
    printf("\t[ESR - Store/AMO Address Misaligned]: \n");
}
void esr_handler_AMO_acc_fault(void)
{
    printf("\t[ESR - Store/AMO Access Fault]: \n");
}

/*
    CUSTOM ECALL CODES

    Ecall code (a7) | Description
            1       | Used to terminate the execution
         address    | Used to check jump instruction
        address + 1 | Used to check return instruction
*/
void esr_handler_U_mode_ecall(unsigned int ecode_address_encoding, unsigned int mepc)
{
    if (ecode_address_encoding == 1)
    {
        code_termination();
    }
    else if ((ecode_address_encoding % 2) == 0) // If the address is even we check for jump
    {
        /*
            CFG CHECK: mepc + 4 and destination address must be legal

            IF ALLOWED PUSH mepc + 4 (ecall) + 2 (jump instruction) to shadow stack
        */
        printf("\t[ESR - U Mode Ecall]:\tJump check requested for %x and mepc: %x\n", ecode_address_encoding, mepc);
        unsigned int source = mepc + 4;
        
        if(check(&cfg, source, ecode_address_encoding) == 1){
            if(push(&shadow_stack, source + 2) != 1)
            {  
                printf("\t[ESR - U Mode Ecall]:\tStack is full, terminating execution ...\n");
                code_termination();
            } 
        } else {
            printf("\t[ESR - U Mode Ecall]:\tNo CFG match, terminating execution ...\n");
            code_termination();
        }

        printf("\t[ESR - U Mode Ecall]:\tReturn address %x stored correctly ...\n", source + 2);
    }
    else if ((ecode_address_encoding % 2) != 0) // If the address is odd, we remove 1 and check for return
    {
        /*
            SHADOW STACK CHECK: destination address and popped address must be equal
        */
        unsigned int stored_address = pop(&shadow_stack);
        printf("\t[ESR - U Mode Ecall]:\tReturn check requested for %x, mepc: %x and popped value: %x\n", ecode_address_encoding - 1, mepc, stored_address);
        
        if (stored_address == 0 || stored_address != ecode_address_encoding - 1)
        {
            printf("\t[ESR - U Mode Ecall]:\tWrong return address or empty stack, terminating execution ...\n");
            code_termination();
        }
        printf("\t[ESR - U Mode Ecall]:\tReturn address is correct, return allowed ...\n");
    } else
    {
        printf("\t[ESR - U Mode Ecall]:\tUndefined Ecall code %d\n", ecode_address_encoding);
    }
}
void esr_handler_S_mode_ecall(void)
{
    printf("\t[ESR - S Mode Ecall]: \n");
}
void esr_handler_M_mode_ecall(unsigned int ecode_address_encoding, unsigned int mepc)
{
    //printf("\t[ESR - M Mode Ecall]: \n");
     if (ecode_address_encoding == 1)
    {
        code_termination();
    }
    else if ((ecode_address_encoding % 2) == 0) // If the address is even we check for jump
    {
        /*
            CFG CHECK: mepc + 4 and destination address must be legal

            IF ALLOWED PUSH mepc + 4 (ecall) + 2 (jump instruction) to shadow stack
        */
        printf("\t[ESR - M Mode Ecall]:\tJump check requested for %x and mepc: %x\n", ecode_address_encoding, mepc);
        unsigned int source = mepc + 4;
        
        if(check(&cfg, source, ecode_address_encoding) == 1){
            if(push(&shadow_stack, source + 2) != 1)
            {  
                printf("\t[ESR - U Mode Ecall]:\tStack is full, terminating execution ...\n");
                code_termination();
            } 
        } else {
            printf("\t[ESR - U Mode Ecall]:\tNo CFG match, terminating execution ...\n");
            code_termination();
        }

        printf("\t[ESR - M Mode Ecall]:\tReturn address %x stored correctly ...\n", source + 2);
    }
    else if ((ecode_address_encoding % 2) != 0) // If the address is odd, we remove 1 and check for return
    {
        /*
            SHADOW STACK CHECK: destination address and popped address must be equal
        */
        unsigned int stored_address = pop(&shadow_stack);
        printf("\t[ESR - M Mode Ecall]:\tReturn check requested for %x, mepc: %x and popped value: %x\n", ecode_address_encoding - 1, mepc, stored_address);
        
        if (stored_address == 0 || stored_address != ecode_address_encoding - 1)
        {
            printf("\t[ESR - M Mode Ecall]:\tWrong return address or empty stack, terminating execution ...\n");
            code_termination();
        }
        printf("\t[ESR - M Mode Ecall]:\tReturn address is correct, return allowed ...\n");
    } else
    {
        printf("\t[ESR - M Mode Ecall]:\tUndefined Ecall code %d\n", ecode_address_encoding);
    }
}
void esr_handler_instr_page_fault(void)
{
    printf("\t[ESR - Instruction Page Fault]: \n");
}
void esr_handler_load_page_fault(void)
{
    printf("\t[ESR - Load Page Fault]: \n");
}
void esr_handler_AMO_page_fault(void)
{
    printf("\t[ESR - Store/AMO Page Fault]: \n");
}
/*
    Should never be called since reserved
*/
void esr_handler_reserved(void)
{
    printf("\t[ESR - Error Reserved]\n");
}

/*
    Interrupt Service Routine functions
*/

void isr_user_software(void)
{
    printf("\t[ISR - User Software Interrupt]:\n");
}
void isr_supervisor_software(void)
{
    printf("\t[ISR - Supervisor Software Interrupt]:\n");
}
void isr_machine_software(void)
{
    printf("\t[ISR - Machine Software Interrupt]:\n");
}
void isr_user_timer(void)
{
    printf("\t[ISR - User Timer Interrupt]:\n");
}
void isr_supervisor_timer(void)
{
    printf("\t[ISR - Supervisor Timer Interrupt]:\n");
}
void isr_machine_timer(void)
{
    printf("\t[ISR - Machine Timer Interrupt]:\n");
}
void isr_user_external(void)
{
    printf("\t[ISR - User External Interrupt]:\n");
}
void isr_supervisor_external(void)
{
    printf("\t[ISR - Supervisor External Interrupt]:\n");
}
void isr_machine_external(void)
{
    printf("\t[ISR - Machine External Interrupt]:\n");
}

/*
    Should never be called since reserved
*/
void isr_reserved(void)
{
    printf("\t[ISR - Error   Reserved]\n");
}

/*
    Function called to load the address of code termination and stop the execution
*/
void code_termination(void)
{
    asm("la t0, terminate_execution");
    asm("csrw mepc, t0");
    asm("mret");
}