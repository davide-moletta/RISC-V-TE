#ifndef INTR_VECTOR_TABLE_H
#define INTR_VECTOR_TABLE_H

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

#endif