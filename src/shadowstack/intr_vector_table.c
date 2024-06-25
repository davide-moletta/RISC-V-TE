#include <stdio.h>

void interrupt_vector_table(void)        __attribute__((section(".intr_vector_table")));

void synchronous_exception_handler(void) __attribute__((section(".intr_service_routines")));
void ecall_handler()                     __attribute__((section(".intr_service_routines")));
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
    asm("add sp, sp, -160\n\t"
        "sw ra, 4(sp)\n\t"
        "sw tp, 16(sp)\n\t"
        "sw t0, 20(sp)\n\t"
        "sw t1, 24(sp)\n\t"
        "sw t2, 28(sp)\n\t"
        "sw s0, 32(sp)\n\t"
        "sw s1, 36(sp)\n\t"
        "sw a0, 40(sp)\n\t"
        "sw a1, 44(sp)\n\t"
        "sw a2, 48(sp)\n\t"
        "sw a3, 52(sp)\n\t"
        "sw a4, 56(sp)\n\t"
        "sw a5, 60(sp)\n\t"
        "sw a6, 64(sp)\n\t"
        "sw a7, 68(sp)\n\t"
        "sw s2, 72(sp)\n\t"
        "sw s3, 76(sp)\n\t"
        "sw s4, 80(sp)\n\t"
        "sw s5, 84(sp)\n\t"
        "sw s6, 88(sp)\n\t"
        "sw s7, 92(sp)\n\t"
        "sw s8, 96(sp)\n\t"
        "sw s9, 100(sp)\n\t"
        "sw s10, 104(sp)\n\t"
        "sw s11, 108(sp)\n\t"
        "sw t3, 112(sp)\n\t"
        "sw t4, 116(sp)\n\t"
        "sw t5, 120(sp)\n\t"
        "sw t6, 124(sp)\n\t"
	);

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
    asm volatile ("csrr %0, mcause" : "=r" (mcause));

	printf("mcause: %ld", mcause);

	// Check the MSB (bit 31) of the mcause register
    if (mcause & 0x80000000) { isr_user_software(); } // If it is set call the user software interrupt

	// Check if it's an environment call from U-mode
    if ((mcause & 0xFF) == 8) { ecall_handler(); } // If it is call the environment call handler

    // Adjust the mepc to point to the next instruction after ecall
    asm("csrr t0, mepc\n\t"
        "addi t0, t0, 4\n\t"
        "csrw mepc, t0\n\t"
	);

    // Restore registers and state
    asm("lw ra, 4(sp)\n\t"
        "lw tp, 16(sp)\n\t"
        "lw t0, 20(sp)\n\t"
        "lw t1, 24(sp)\n\t"
        "lw t2, 28(sp)\n\t"
        "lw s0, 32(sp)\n\t"
        "lw s1, 36(sp)\n\t"
        "lw a0, 40(sp)\n\t"
        "lw a1, 44(sp)\n\t"
        "lw a2, 48(sp)\n\t"
        "lw a3, 52(sp)\n\t"
        "lw a4, 56(sp)\n\t"
        "lw a5, 60(sp)\n\t"
        "lw a6, 64(sp)\n\t"
        "lw a7, 68(sp)\n\t"
        "lw s2, 72(sp)\n\t"
        "lw s3, 76(sp)\n\t"
        "lw s4, 80(sp)\n\t"
        "lw s5, 84(sp)\n\t"
        "lw s6, 88(sp)\n\t"
        "lw s7, 92(sp)\n\t"
        "lw s8, 96(sp)\n\t"
        "lw s9, 100(sp)\n\t"
        "lw s10, 104(sp)\n\t"
        "lw s11, 108(sp)\n\t"
        "lw t3, 112(sp)\n\t"
        "lw t4, 116(sp)\n\t"
        "lw t5, 120(sp)\n\t"
        "lw t6, 124(sp)\n\t"
        "add sp, sp, 160\n\t"
    );

	asm("mret");
}

void ecall_handler()
{
	// Save state
    asm("add sp, sp, -16\n\t"
        "sw ra, 12(sp)\n\t"
        "sw s0, 8(sp)\n\t"
	);

    printf("Woah, you made an environment call\n");

	/*
		HANDLE ECALL
	*/

	// Restore state
    asm("lw ra, 12(sp)\n\t"
        "lw s0, 8(sp)\n\t"
        "add sp, sp, 16\n\t"
	);

    // Return to synchronous_exception_handler to continue processing
    asm("ret");
}

void isr_user_software(void)
{
    printf("User software interrupt generated");
    // Open stack pointer and save values
    asm("add sp, sp, -128\n\t"
        "sw	ra, 0(sp)\n\t"
        "sw	gp, 4(sp)\n\t"
	    "sw	tp, 8(sp)\n\t"
	    "sw	t0, 12(sp)\n\t"
	    "sw	t1, 16(sp)\n\t"
	    "sw	t2, 20(sp)\n\t"
	    "sw	s0, 24(sp)\n\t"
	    "sw	s1, 28(sp)\n\t"
	    "sw	a0, 32(sp)\n\t"
	    "sw	a1, 36(sp)\n\t"
	    "sw	a2, 40(sp)\n\t"
	    "sw	a3, 44(sp)\n\t"
	    "sw	a4, 48(sp)\n\t"
	    "sw	a5, 52(sp)\n\t"
	    "sw	a6, 56(sp)\n\t"
	    "sw	a7, 60(sp)\n\t"
	    "sw	s2, 64(sp)\n\t"
	    "sw	s3, 68(sp)\n\t"
	    "sw	s4, 72(sp)\n\t"
	    "sw	s5, 76(sp)\n\t"
	    "sw	s6, 80(sp)\n\t"
	    "sw	s7, 84(sp)\n\t"
	    "sw	s8, 88(sp)\n\t"
	    "sw	s9, 92(sp)\n\t"
	    "sw	s10, 96(sp)\n\t"
	    "sw	s11, 100(sp)\n\t"
	    "sw	t3, 104(sp)\n\t"
	    "sw	t4, 108(sp)\n\t"
	    "sw	t5, 112(sp)\n\t"
	    "sw	t6, 116(sp)\n\t");

    /*
        Put here your handling routine
    */

    // Restore values and close stack pointer
    asm("lw	ra, 0(sp)\n\t"
        "lw	gp, 4(sp)\n\t"
	    "lw	tp, 8(sp)\n\t"
	    "lw	t0, 12(sp)\n\t"
	    "lw	t1, 16(sp)\n\t"
	    "lw	t2, 20(sp)\n\t"
	    "lw	s0, 24(sp)\n\t"
	    "lw	s1, 28(sp)\n\t"
	    "lw	a0, 32(sp)\n\t"
	    "lw	a1, 36(sp)\n\t"
	    "lw	a2, 40(sp)\n\t"
	    "lw	a3, 44(sp)\n\t"
	    "lw	a4, 48(sp)\n\t"
	    "lw	a5, 52(sp)\n\t"
	    "lw	a6, 56(sp)\n\t"
	    "lw	a7, 60(sp)\n\t"
	    "lw	s2, 64(sp)\n\t"
	    "lw	s3, 68(sp)\n\t"
	    "lw	s4, 72(sp)\n\t"
	    "lw	s5, 76(sp)\n\t"
	    "lw	s6, 80(sp)\n\t"
	    "lw	s7, 84(sp)\n\t"
	    "lw	s8, 88(sp)\n\t"
	    "lw	s9, 92(sp)\n\t"
	    "lw	s10, 96(sp)\n\t"
	    "lw	s11, 100(sp)\n\t"
	    "lw	t3, 104(sp)\n\t"
	    "lw	t4, 108(sp)\n\t"
	    "lw	t5, 112(sp)\n\t"
	    "lw	t6, 116(sp)\n\t"
        "add sp, sp, 128\n\t");

    asm("mret");
}
void isr_supervisor_software(void)
{
    printf("Supervisor software interrupt generated");
    // Open stack pointer and save values
    asm("add sp, sp, -128\n\t"
        "sw	ra, 0(sp)\n\t"
        "sw	gp, 4(sp)\n\t"
	    "sw	tp, 8(sp)\n\t"
	    "sw	t0, 12(sp)\n\t"
	    "sw	t1, 16(sp)\n\t"
	    "sw	t2, 20(sp)\n\t"
	    "sw	s0, 24(sp)\n\t"
	    "sw	s1, 28(sp)\n\t"
	    "sw	a0, 32(sp)\n\t"
	    "sw	a1, 36(sp)\n\t"
	    "sw	a2, 40(sp)\n\t"
	    "sw	a3, 44(sp)\n\t"
	    "sw	a4, 48(sp)\n\t"
	    "sw	a5, 52(sp)\n\t"
	    "sw	a6, 56(sp)\n\t"
	    "sw	a7, 60(sp)\n\t"
	    "sw	s2, 64(sp)\n\t"
	    "sw	s3, 68(sp)\n\t"
	    "sw	s4, 72(sp)\n\t"
	    "sw	s5, 76(sp)\n\t"
	    "sw	s6, 80(sp)\n\t"
	    "sw	s7, 84(sp)\n\t"
	    "sw	s8, 88(sp)\n\t"
	    "sw	s9, 92(sp)\n\t"
	    "sw	s10, 96(sp)\n\t"
	    "sw	s11, 100(sp)\n\t"
	    "sw	t3, 104(sp)\n\t"
	    "sw	t4, 108(sp)\n\t"
	    "sw	t5, 112(sp)\n\t"
	    "sw	t6, 116(sp)\n\t");

    /*
        Put here your handling routine
    */

    // Restore values and close stack pointer
    asm("lw	ra, 0(sp)\n\t"
        "lw	gp, 4(sp)\n\t"
	    "lw	tp, 8(sp)\n\t"
	    "lw	t0, 12(sp)\n\t"
	    "lw	t1, 16(sp)\n\t"
	    "lw	t2, 20(sp)\n\t"
	    "lw	s0, 24(sp)\n\t"
	    "lw	s1, 28(sp)\n\t"
	    "lw	a0, 32(sp)\n\t"
	    "lw	a1, 36(sp)\n\t"
	    "lw	a2, 40(sp)\n\t"
	    "lw	a3, 44(sp)\n\t"
	    "lw	a4, 48(sp)\n\t"
	    "lw	a5, 52(sp)\n\t"
	    "lw	a6, 56(sp)\n\t"
	    "lw	a7, 60(sp)\n\t"
	    "lw	s2, 64(sp)\n\t"
	    "lw	s3, 68(sp)\n\t"
	    "lw	s4, 72(sp)\n\t"
	    "lw	s5, 76(sp)\n\t"
	    "lw	s6, 80(sp)\n\t"
	    "lw	s7, 84(sp)\n\t"
	    "lw	s8, 88(sp)\n\t"
	    "lw	s9, 92(sp)\n\t"
	    "lw	s10, 96(sp)\n\t"
	    "lw	s11, 100(sp)\n\t"
	    "lw	t3, 104(sp)\n\t"
	    "lw	t4, 108(sp)\n\t"
	    "lw	t5, 112(sp)\n\t"
	    "lw	t6, 116(sp)\n\t"
        "add sp, sp, 128\n\t");

    asm("mret");
}
void isr_machine_software(void)
{
    printf("Machine software interrupt generated\n");
    // Open stack pointer and save values
    asm("add sp, sp, -128\n\t"
        "sw	ra, 0(sp)\n\t"
        "sw	gp, 4(sp)\n\t"
	    "sw	tp, 8(sp)\n\t"
	    "sw	t0, 12(sp)\n\t"
	    "sw	t1, 16(sp)\n\t"
	    "sw	t2, 20(sp)\n\t"
	    "sw	s0, 24(sp)\n\t"
	    "sw	s1, 28(sp)\n\t"
	    "sw	a0, 32(sp)\n\t"
	    "sw	a1, 36(sp)\n\t"
	    "sw	a2, 40(sp)\n\t"
	    "sw	a3, 44(sp)\n\t"
	    "sw	a4, 48(sp)\n\t"
	    "sw	a5, 52(sp)\n\t"
	    "sw	a6, 56(sp)\n\t"
	    "sw	a7, 60(sp)\n\t"
	    "sw	s2, 64(sp)\n\t"
	    "sw	s3, 68(sp)\n\t"
	    "sw	s4, 72(sp)\n\t"
	    "sw	s5, 76(sp)\n\t"
	    "sw	s6, 80(sp)\n\t"
	    "sw	s7, 84(sp)\n\t"
	    "sw	s8, 88(sp)\n\t"
	    "sw	s9, 92(sp)\n\t"
	    "sw	s10, 96(sp)\n\t"
	    "sw	s11, 100(sp)\n\t"
	    "sw	t3, 104(sp)\n\t"
	    "sw	t4, 108(sp)\n\t"
	    "sw	t5, 112(sp)\n\t"
	    "sw	t6, 116(sp)\n\t");

    /*
        Put here your handling routine
    */

    // Restore values and close stack pointer
    asm("lw	ra, 0(sp)\n\t"
        "lw	gp, 4(sp)\n\t"
	    "lw	tp, 8(sp)\n\t"
	    "lw	t0, 12(sp)\n\t"
	    "lw	t1, 16(sp)\n\t"
	    "lw	t2, 20(sp)\n\t"
	    "lw	s0, 24(sp)\n\t"
	    "lw	s1, 28(sp)\n\t"
	    "lw	a0, 32(sp)\n\t"
	    "lw	a1, 36(sp)\n\t"
	    "lw	a2, 40(sp)\n\t"
	    "lw	a3, 44(sp)\n\t"
	    "lw	a4, 48(sp)\n\t"
	    "lw	a5, 52(sp)\n\t"
	    "lw	a6, 56(sp)\n\t"
	    "lw	a7, 60(sp)\n\t"
	    "lw	s2, 64(sp)\n\t"
	    "lw	s3, 68(sp)\n\t"
	    "lw	s4, 72(sp)\n\t"
	    "lw	s5, 76(sp)\n\t"
	    "lw	s6, 80(sp)\n\t"
	    "lw	s7, 84(sp)\n\t"
	    "lw	s8, 88(sp)\n\t"
	    "lw	s9, 92(sp)\n\t"
	    "lw	s10, 96(sp)\n\t"
	    "lw	s11, 100(sp)\n\t"
	    "lw	t3, 104(sp)\n\t"
	    "lw	t4, 108(sp)\n\t"
	    "lw	t5, 112(sp)\n\t"
	    "lw	t6, 116(sp)\n\t"
        "add sp, sp, 128\n\t");

    asm("mret");
}
void isr_user_timer(void)
{
    printf("User timer interrupt generated\n");
    // Open stack pointer and save values
    asm("add sp, sp, -128\n\t"
        "sw	ra, 0(sp)\n\t"
        "sw	gp, 4(sp)\n\t"
	    "sw	tp, 8(sp)\n\t"
	    "sw	t0, 12(sp)\n\t"
	    "sw	t1, 16(sp)\n\t"
	    "sw	t2, 20(sp)\n\t"
	    "sw	s0, 24(sp)\n\t"
	    "sw	s1, 28(sp)\n\t"
	    "sw	a0, 32(sp)\n\t"
	    "sw	a1, 36(sp)\n\t"
	    "sw	a2, 40(sp)\n\t"
	    "sw	a3, 44(sp)\n\t"
	    "sw	a4, 48(sp)\n\t"
	    "sw	a5, 52(sp)\n\t"
	    "sw	a6, 56(sp)\n\t"
	    "sw	a7, 60(sp)\n\t"
	    "sw	s2, 64(sp)\n\t"
	    "sw	s3, 68(sp)\n\t"
	    "sw	s4, 72(sp)\n\t"
	    "sw	s5, 76(sp)\n\t"
	    "sw	s6, 80(sp)\n\t"
	    "sw	s7, 84(sp)\n\t"
	    "sw	s8, 88(sp)\n\t"
	    "sw	s9, 92(sp)\n\t"
	    "sw	s10, 96(sp)\n\t"
	    "sw	s11, 100(sp)\n\t"
	    "sw	t3, 104(sp)\n\t"
	    "sw	t4, 108(sp)\n\t"
	    "sw	t5, 112(sp)\n\t"
	    "sw	t6, 116(sp)\n\t");

    /*
        Put here your handling routine
    */

    // Restore values and close stack pointer
    asm("lw	ra, 0(sp)\n\t"
        "lw	gp, 4(sp)\n\t"
	    "lw	tp, 8(sp)\n\t"
	    "lw	t0, 12(sp)\n\t"
	    "lw	t1, 16(sp)\n\t"
	    "lw	t2, 20(sp)\n\t"
	    "lw	s0, 24(sp)\n\t"
	    "lw	s1, 28(sp)\n\t"
	    "lw	a0, 32(sp)\n\t"
	    "lw	a1, 36(sp)\n\t"
	    "lw	a2, 40(sp)\n\t"
	    "lw	a3, 44(sp)\n\t"
	    "lw	a4, 48(sp)\n\t"
	    "lw	a5, 52(sp)\n\t"
	    "lw	a6, 56(sp)\n\t"
	    "lw	a7, 60(sp)\n\t"
	    "lw	s2, 64(sp)\n\t"
	    "lw	s3, 68(sp)\n\t"
	    "lw	s4, 72(sp)\n\t"
	    "lw	s5, 76(sp)\n\t"
	    "lw	s6, 80(sp)\n\t"
	    "lw	s7, 84(sp)\n\t"
	    "lw	s8, 88(sp)\n\t"
	    "lw	s9, 92(sp)\n\t"
	    "lw	s10, 96(sp)\n\t"
	    "lw	s11, 100(sp)\n\t"
	    "lw	t3, 104(sp)\n\t"
	    "lw	t4, 108(sp)\n\t"
	    "lw	t5, 112(sp)\n\t"
	    "lw	t6, 116(sp)\n\t"
        "add sp, sp, 128\n\t");

    asm("mret");
}
void isr_supervisor_timer(void)
{
    printf("Supervisor timer interrupt generated");
    // Open stack pointer and save values
    asm("add sp, sp, -128\n\t"
        "sw	ra, 0(sp)\n\t"
        "sw	gp, 4(sp)\n\t"
	    "sw	tp, 8(sp)\n\t"
	    "sw	t0, 12(sp)\n\t"
	    "sw	t1, 16(sp)\n\t"
	    "sw	t2, 20(sp)\n\t"
	    "sw	s0, 24(sp)\n\t"
	    "sw	s1, 28(sp)\n\t"
	    "sw	a0, 32(sp)\n\t"
	    "sw	a1, 36(sp)\n\t"
	    "sw	a2, 40(sp)\n\t"
	    "sw	a3, 44(sp)\n\t"
	    "sw	a4, 48(sp)\n\t"
	    "sw	a5, 52(sp)\n\t"
	    "sw	a6, 56(sp)\n\t"
	    "sw	a7, 60(sp)\n\t"
	    "sw	s2, 64(sp)\n\t"
	    "sw	s3, 68(sp)\n\t"
	    "sw	s4, 72(sp)\n\t"
	    "sw	s5, 76(sp)\n\t"
	    "sw	s6, 80(sp)\n\t"
	    "sw	s7, 84(sp)\n\t"
	    "sw	s8, 88(sp)\n\t"
	    "sw	s9, 92(sp)\n\t"
	    "sw	s10, 96(sp)\n\t"
	    "sw	s11, 100(sp)\n\t"
	    "sw	t3, 104(sp)\n\t"
	    "sw	t4, 108(sp)\n\t"
	    "sw	t5, 112(sp)\n\t"
	    "sw	t6, 116(sp)\n\t");

    /*
        Put here your handling routine
    */

    // Restore values and close stack pointer
    asm("lw	ra, 0(sp)\n\t"
        "lw	gp, 4(sp)\n\t"
	    "lw	tp, 8(sp)\n\t"
	    "lw	t0, 12(sp)\n\t"
	    "lw	t1, 16(sp)\n\t"
	    "lw	t2, 20(sp)\n\t"
	    "lw	s0, 24(sp)\n\t"
	    "lw	s1, 28(sp)\n\t"
	    "lw	a0, 32(sp)\n\t"
	    "lw	a1, 36(sp)\n\t"
	    "lw	a2, 40(sp)\n\t"
	    "lw	a3, 44(sp)\n\t"
	    "lw	a4, 48(sp)\n\t"
	    "lw	a5, 52(sp)\n\t"
	    "lw	a6, 56(sp)\n\t"
	    "lw	a7, 60(sp)\n\t"
	    "lw	s2, 64(sp)\n\t"
	    "lw	s3, 68(sp)\n\t"
	    "lw	s4, 72(sp)\n\t"
	    "lw	s5, 76(sp)\n\t"
	    "lw	s6, 80(sp)\n\t"
	    "lw	s7, 84(sp)\n\t"
	    "lw	s8, 88(sp)\n\t"
	    "lw	s9, 92(sp)\n\t"
	    "lw	s10, 96(sp)\n\t"
	    "lw	s11, 100(sp)\n\t"
	    "lw	t3, 104(sp)\n\t"
	    "lw	t4, 108(sp)\n\t"
	    "lw	t5, 112(sp)\n\t"
	    "lw	t6, 116(sp)\n\t"
        "add sp, sp, 128\n\t");

    asm("mret");
}
void isr_machine_timer(void)
{
    printf("Machine timer interrupt generated\n");
    // Open stack pointer and save values
    asm("add sp, sp, -128\n\t"
        "sw	ra, 0(sp)\n\t"
        "sw	gp, 4(sp)\n\t"
	    "sw	tp, 8(sp)\n\t"
	    "sw	t0, 12(sp)\n\t"
	    "sw	t1, 16(sp)\n\t"
	    "sw	t2, 20(sp)\n\t"
	    "sw	s0, 24(sp)\n\t"
	    "sw	s1, 28(sp)\n\t"
	    "sw	a0, 32(sp)\n\t"
	    "sw	a1, 36(sp)\n\t"
	    "sw	a2, 40(sp)\n\t"
	    "sw	a3, 44(sp)\n\t"
	    "sw	a4, 48(sp)\n\t"
	    "sw	a5, 52(sp)\n\t"
	    "sw	a6, 56(sp)\n\t"
	    "sw	a7, 60(sp)\n\t"
	    "sw	s2, 64(sp)\n\t"
	    "sw	s3, 68(sp)\n\t"
	    "sw	s4, 72(sp)\n\t"
	    "sw	s5, 76(sp)\n\t"
	    "sw	s6, 80(sp)\n\t"
	    "sw	s7, 84(sp)\n\t"
	    "sw	s8, 88(sp)\n\t"
	    "sw	s9, 92(sp)\n\t"
	    "sw	s10, 96(sp)\n\t"
	    "sw	s11, 100(sp)\n\t"
	    "sw	t3, 104(sp)\n\t"
	    "sw	t4, 108(sp)\n\t"
	    "sw	t5, 112(sp)\n\t"
	    "sw	t6, 116(sp)\n\t");

    /*
        Put here your handling routine
    */

    // Restore values and close stack pointer
    asm("lw	ra, 0(sp)\n\t"
        "lw	gp, 4(sp)\n\t"
	    "lw	tp, 8(sp)\n\t"
	    "lw	t0, 12(sp)\n\t"
	    "lw	t1, 16(sp)\n\t"
	    "lw	t2, 20(sp)\n\t"
	    "lw	s0, 24(sp)\n\t"
	    "lw	s1, 28(sp)\n\t"
	    "lw	a0, 32(sp)\n\t"
	    "lw	a1, 36(sp)\n\t"
	    "lw	a2, 40(sp)\n\t"
	    "lw	a3, 44(sp)\n\t"
	    "lw	a4, 48(sp)\n\t"
	    "lw	a5, 52(sp)\n\t"
	    "lw	a6, 56(sp)\n\t"
	    "lw	a7, 60(sp)\n\t"
	    "lw	s2, 64(sp)\n\t"
	    "lw	s3, 68(sp)\n\t"
	    "lw	s4, 72(sp)\n\t"
	    "lw	s5, 76(sp)\n\t"
	    "lw	s6, 80(sp)\n\t"
	    "lw	s7, 84(sp)\n\t"
	    "lw	s8, 88(sp)\n\t"
	    "lw	s9, 92(sp)\n\t"
	    "lw	s10, 96(sp)\n\t"
	    "lw	s11, 100(sp)\n\t"
	    "lw	t3, 104(sp)\n\t"
	    "lw	t4, 108(sp)\n\t"
	    "lw	t5, 112(sp)\n\t"
	    "lw	t6, 116(sp)\n\t"
        "add sp, sp, 128\n\t");

    asm("mret");
}
void isr_user_external(void)
{
    printf("User external interrupt generated");
    // Open stack pointer and save values
    asm("add sp, sp, -128\n\t"
        "sw	ra, 0(sp)\n\t"
        "sw	gp, 4(sp)\n\t"
	    "sw	tp, 8(sp)\n\t"
	    "sw	t0, 12(sp)\n\t"
	    "sw	t1, 16(sp)\n\t"
	    "sw	t2, 20(sp)\n\t"
	    "sw	s0, 24(sp)\n\t"
	    "sw	s1, 28(sp)\n\t"
	    "sw	a0, 32(sp)\n\t"
	    "sw	a1, 36(sp)\n\t"
	    "sw	a2, 40(sp)\n\t"
	    "sw	a3, 44(sp)\n\t"
	    "sw	a4, 48(sp)\n\t"
	    "sw	a5, 52(sp)\n\t"
	    "sw	a6, 56(sp)\n\t"
	    "sw	a7, 60(sp)\n\t"
	    "sw	s2, 64(sp)\n\t"
	    "sw	s3, 68(sp)\n\t"
	    "sw	s4, 72(sp)\n\t"
	    "sw	s5, 76(sp)\n\t"
	    "sw	s6, 80(sp)\n\t"
	    "sw	s7, 84(sp)\n\t"
	    "sw	s8, 88(sp)\n\t"
	    "sw	s9, 92(sp)\n\t"
	    "sw	s10, 96(sp)\n\t"
	    "sw	s11, 100(sp)\n\t"
	    "sw	t3, 104(sp)\n\t"
	    "sw	t4, 108(sp)\n\t"
	    "sw	t5, 112(sp)\n\t"
	    "sw	t6, 116(sp)\n\t");

    /*
        Put here your handling routine
    */

    // Restore values and close stack pointer
    asm("lw	ra, 0(sp)\n\t"
        "lw	gp, 4(sp)\n\t"
	    "lw	tp, 8(sp)\n\t"
	    "lw	t0, 12(sp)\n\t"
	    "lw	t1, 16(sp)\n\t"
	    "lw	t2, 20(sp)\n\t"
	    "lw	s0, 24(sp)\n\t"
	    "lw	s1, 28(sp)\n\t"
	    "lw	a0, 32(sp)\n\t"
	    "lw	a1, 36(sp)\n\t"
	    "lw	a2, 40(sp)\n\t"
	    "lw	a3, 44(sp)\n\t"
	    "lw	a4, 48(sp)\n\t"
	    "lw	a5, 52(sp)\n\t"
	    "lw	a6, 56(sp)\n\t"
	    "lw	a7, 60(sp)\n\t"
	    "lw	s2, 64(sp)\n\t"
	    "lw	s3, 68(sp)\n\t"
	    "lw	s4, 72(sp)\n\t"
	    "lw	s5, 76(sp)\n\t"
	    "lw	s6, 80(sp)\n\t"
	    "lw	s7, 84(sp)\n\t"
	    "lw	s8, 88(sp)\n\t"
	    "lw	s9, 92(sp)\n\t"
	    "lw	s10, 96(sp)\n\t"
	    "lw	s11, 100(sp)\n\t"
	    "lw	t3, 104(sp)\n\t"
	    "lw	t4, 108(sp)\n\t"
	    "lw	t5, 112(sp)\n\t"
	    "lw	t6, 116(sp)\n\t"
        "add sp, sp, 128\n\t");

    asm("mret");
}
void isr_supervisor_external(void)
{
    printf("Supervisor external interrupt generated");
    // Open stack pointer and save values
    asm("add sp, sp, -128\n\t"
        "sw	ra, 0(sp)\n\t"
        "sw	gp, 4(sp)\n\t"
	    "sw	tp, 8(sp)\n\t"
	    "sw	t0, 12(sp)\n\t"
	    "sw	t1, 16(sp)\n\t"
	    "sw	t2, 20(sp)\n\t"
	    "sw	s0, 24(sp)\n\t"
	    "sw	s1, 28(sp)\n\t"
	    "sw	a0, 32(sp)\n\t"
	    "sw	a1, 36(sp)\n\t"
	    "sw	a2, 40(sp)\n\t"
	    "sw	a3, 44(sp)\n\t"
	    "sw	a4, 48(sp)\n\t"
	    "sw	a5, 52(sp)\n\t"
	    "sw	a6, 56(sp)\n\t"
	    "sw	a7, 60(sp)\n\t"
	    "sw	s2, 64(sp)\n\t"
	    "sw	s3, 68(sp)\n\t"
	    "sw	s4, 72(sp)\n\t"
	    "sw	s5, 76(sp)\n\t"
	    "sw	s6, 80(sp)\n\t"
	    "sw	s7, 84(sp)\n\t"
	    "sw	s8, 88(sp)\n\t"
	    "sw	s9, 92(sp)\n\t"
	    "sw	s10, 96(sp)\n\t"
	    "sw	s11, 100(sp)\n\t"
	    "sw	t3, 104(sp)\n\t"
	    "sw	t4, 108(sp)\n\t"
	    "sw	t5, 112(sp)\n\t"
	    "sw	t6, 116(sp)\n\t");

    /*
        Put here your handling routine
    */

    // Restore values and close stack pointer
    asm("lw	ra, 0(sp)\n\t"
        "lw	gp, 4(sp)\n\t"
	    "lw	tp, 8(sp)\n\t"
	    "lw	t0, 12(sp)\n\t"
	    "lw	t1, 16(sp)\n\t"
	    "lw	t2, 20(sp)\n\t"
	    "lw	s0, 24(sp)\n\t"
	    "lw	s1, 28(sp)\n\t"
	    "lw	a0, 32(sp)\n\t"
	    "lw	a1, 36(sp)\n\t"
	    "lw	a2, 40(sp)\n\t"
	    "lw	a3, 44(sp)\n\t"
	    "lw	a4, 48(sp)\n\t"
	    "lw	a5, 52(sp)\n\t"
	    "lw	a6, 56(sp)\n\t"
	    "lw	a7, 60(sp)\n\t"
	    "lw	s2, 64(sp)\n\t"
	    "lw	s3, 68(sp)\n\t"
	    "lw	s4, 72(sp)\n\t"
	    "lw	s5, 76(sp)\n\t"
	    "lw	s6, 80(sp)\n\t"
	    "lw	s7, 84(sp)\n\t"
	    "lw	s8, 88(sp)\n\t"
	    "lw	s9, 92(sp)\n\t"
	    "lw	s10, 96(sp)\n\t"
	    "lw	s11, 100(sp)\n\t"
	    "lw	t3, 104(sp)\n\t"
	    "lw	t4, 108(sp)\n\t"
	    "lw	t5, 112(sp)\n\t"
	    "lw	t6, 116(sp)\n\t"
        "add sp, sp, 128\n\t");

    asm("mret");
}
void isr_machine_external(void)
{
    printf("Machine external interrupt generated");
    // Open stack pointer and save values
    asm("add sp, sp, -128\n\t"
        "sw	ra, 0(sp)\n\t"
        "sw	gp, 4(sp)\n\t"
	    "sw	tp, 8(sp)\n\t"
	    "sw	t0, 12(sp)\n\t"
	    "sw	t1, 16(sp)\n\t"
	    "sw	t2, 20(sp)\n\t"
	    "sw	s0, 24(sp)\n\t"
	    "sw	s1, 28(sp)\n\t"
	    "sw	a0, 32(sp)\n\t"
	    "sw	a1, 36(sp)\n\t"
	    "sw	a2, 40(sp)\n\t"
	    "sw	a3, 44(sp)\n\t"
	    "sw	a4, 48(sp)\n\t"
	    "sw	a5, 52(sp)\n\t"
	    "sw	a6, 56(sp)\n\t"
	    "sw	a7, 60(sp)\n\t"
	    "sw	s2, 64(sp)\n\t"
	    "sw	s3, 68(sp)\n\t"
	    "sw	s4, 72(sp)\n\t"
	    "sw	s5, 76(sp)\n\t"
	    "sw	s6, 80(sp)\n\t"
	    "sw	s7, 84(sp)\n\t"
	    "sw	s8, 88(sp)\n\t"
	    "sw	s9, 92(sp)\n\t"
	    "sw	s10, 96(sp)\n\t"
	    "sw	s11, 100(sp)\n\t"
	    "sw	t3, 104(sp)\n\t"
	    "sw	t4, 108(sp)\n\t"
	    "sw	t5, 112(sp)\n\t"
	    "sw	t6, 116(sp)\n\t");

    /*
        Put here your handling routine
    */

    // Restore values and close stack pointer
    asm("lw	ra, 0(sp)\n\t"
        "lw	gp, 4(sp)\n\t"
	    "lw	tp, 8(sp)\n\t"
	    "lw	t0, 12(sp)\n\t"
	    "lw	t1, 16(sp)\n\t"
	    "lw	t2, 20(sp)\n\t"
	    "lw	s0, 24(sp)\n\t"
	    "lw	s1, 28(sp)\n\t"
	    "lw	a0, 32(sp)\n\t"
	    "lw	a1, 36(sp)\n\t"
	    "lw	a2, 40(sp)\n\t"
	    "lw	a3, 44(sp)\n\t"
	    "lw	a4, 48(sp)\n\t"
	    "lw	a5, 52(sp)\n\t"
	    "lw	a6, 56(sp)\n\t"
	    "lw	a7, 60(sp)\n\t"
	    "lw	s2, 64(sp)\n\t"
	    "lw	s3, 68(sp)\n\t"
	    "lw	s4, 72(sp)\n\t"
	    "lw	s5, 76(sp)\n\t"
	    "lw	s6, 80(sp)\n\t"
	    "lw	s7, 84(sp)\n\t"
	    "lw	s8, 88(sp)\n\t"
	    "lw	s9, 92(sp)\n\t"
	    "lw	s10, 96(sp)\n\t"
	    "lw	s11, 100(sp)\n\t"
	    "lw	t3, 104(sp)\n\t"
	    "lw	t4, 108(sp)\n\t"
	    "lw	t5, 112(sp)\n\t"
	    "lw	t6, 116(sp)\n\t"
        "add sp, sp, 128\n\t");

    asm("mret");
}

/*
    Should never be called since reserved
*/
void isr_reserved(void)
{
    printf("\n\n Error: reserved interrupt code used \n\n");
}
