#include <stdio.h>

void interrupt_vector_table(void)        __attribute__((section(".intr_vector_table")));

void synchronous_exception_handler(void) __attribute__((section(".intr_service_routines")));
void ecall_handler(void)                 __attribute__((section(".intr_service_routines")));
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
    // Save registers and state
    // asm("add sp, sp, -116");
    // asm("sw ra, 0(sp)");
    // asm("sw tp, 4(sp)");
    // asm("sw t0, 8(sp)");
    // asm("sw t1, 12(sp)");
    // asm("sw t2, 16(sp)");
    // asm("sw s0, 20(sp)");
    // asm("sw s1, 24(sp)");
    // asm("sw a0, 28(sp)");
    // asm("sw a1, 32(sp)");
    // asm("sw a2, 36(sp)");
    // asm("sw a3, 40(sp)");
    // asm("sw a4, 44(sp)");
    // asm("sw a5, 48(sp)");
    // asm("sw a6, 52(sp)");
    // asm("sw a7, 56(sp)");
    // asm("sw s2, 60(sp)");
    // asm("sw s3, 64(sp)");
    // asm("sw s4, 68(sp)");
    // asm("sw s5, 72(sp)");
    // asm("sw s6, 76(sp)");
    // asm("sw s7, 80(sp)");
    // asm("sw s8, 84(sp)");
    // asm("sw s9, 88(sp)");
    // asm("sw s10, 92(sp)");
    // asm("sw s11, 96(sp)");
    // asm("sw t3, 100(sp)");
    // asm("sw t4, 104(sp)");
    // asm("sw t5, 108(sp)");
    // asm("sw t6, 112(sp)");

    /*
        MCAUSE CSR Shown as

        bit(s) position
        ---------------
        | bit(s) name |
        ---------------
        bit(s) length


        31          30               0
        ------------------------------
        | Interrupt | Exception Code |
        ------------------------------
             1             31

        Interrupt is a single bit set to 1 for interrupts and to 0 for exceptions
        Exception code contains the code of what triggered the exception/interrupt
    */

    // Check the cause of the exception
    unsigned long mcause;
    asm volatile("csrr %0, mcause" : "=r"(mcause));

    // Check the MSB (bit 31) of the mcause register
    if (mcause & 0x80000000) { isr_user_software(); } // If it is set call the user software interrupt

    // Check if it's an environment call from U-mode
    if ((mcause & 0xFF) == 8)
    {
        unsigned long a0;
        asm volatile ("add %0, a0, x0" : "=r" (a0));

        // if a0 contains 1 we terminate the execution
        if(a0 == 1) 
        {
            asm("la t0, terminate_execution");
            asm("csrw mepc, t0");
            asm("mret");
        }
        
        ecall_handler(); // If it is call the environment call handler
    } 

    /*
        MANAGE OTHER CAUSES
    */

    // Adjust the mepc to point to the next instruction after ecall
    asm("csrr t0, mepc\n\t"
        "addi t0, t0, 4\n\t"
        "csrw mepc, t0\n\t");

    // Restore registers and state
    // asm("lw ra, 0(sp)");
    // asm("lw tp, 4(sp)");
    // asm("lw t0, 8(sp)");
    // asm("lw t1, 12(sp)");
    // asm("lw t2, 16(sp)");
    // asm("lw s0, 20(sp)");
    // asm("lw s1, 24(sp)");
    // asm("lw a0, 28(sp)");
    // asm("lw a1, 32(sp)");
    // asm("lw a2, 36(sp)");
    // asm("lw a3, 40(sp)");
    // asm("lw a4, 44(sp)");
    // asm("lw a5, 48(sp)");
    // asm("lw a6, 52(sp)");
    // asm("lw a7, 56(sp)");
    // asm("lw s2, 60(sp)");
    // asm("lw s3, 64(sp)");
    // asm("lw s4, 68(sp)");
    // asm("lw s5, 72(sp)");
    // asm("lw s6, 76(sp)");
    // asm("lw s7, 80(sp)");
    // asm("lw s8, 84(sp)");
    // asm("lw s9, 88(sp)");
    // asm("lw s10, 92(sp)");
    // asm("lw s11, 96(sp)");
    // asm("lw t3, 100(sp)");
    // asm("lw t4, 104(sp)");
    // asm("lw t5, 108(sp)");
    // asm("lw t6, 112(sp)");
    // asm("add sp, sp, 116");

    asm("mret");
}

void ecall_handler(void)
{
    // Save state
    // asm("add sp, sp, -16\n\t"
    //     "sw ra, 12(sp)\n\t"
    //     "sw s0, 8(sp)\n\t"
    // );

    printf("Handling ecall\n");

    // /*
    // 	HANDLE ECALL
    // */

    // // Restore state
    // asm("lw ra, 12(sp)\n\t"
    //     "lw s0, 8(sp)\n\t"
    //     "add sp, sp, 16\n\t"
    // );

    // Return to synchronous_exception_handler to continue processing
    // asm("ret");
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
