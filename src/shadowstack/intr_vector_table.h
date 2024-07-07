#ifndef INTR_VECTOR_TABLE_H
#define INTR_VECTOR_TABLE_H

// Interrupt vector table with all the calls to interrupt service routines
void interrupt_vector_table(void)        __attribute__((section(".intr_vector_table")));

// Interrupt Service Routines to handle interrupts
void synchronous_esreption_handler(void) __attribute__((section(".intr_service_routines")));
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

#endif