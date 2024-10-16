#include <mdk.h>
#include "intr_vector_table.h"
#include "usercode/user_entry.h"

int main(void)                 __attribute__((section(".machine_setup")));
void terminate_execution(void) __attribute__((section(".machine_setup")));

int main(void)
{
     wdt_disable(); // Disable watchdog to avoid continuous resets

     printf("\n\nConfiguring interrupt vector table ...\n");

     asm("la t0, interrupt_vector_table"); // Load vector table address
     asm("ori t0, t0, 1");                 // Set MODE bit to 1 to enable vectored interrupts
     asm("csrw mtvec, t0");                // Load the address in MTVEC

     /*
         MSTATUS CSR Shown as

         bit(s) position
         ---------------
         | bit(s) name |
         ---------------
         bit(s) length


           31 30    23  22   21    20    19   18     17  16      15 14     13 12       11
         --------------------------------------------------------------------------------
         | SD | WPRI | TSR | TW | TVM | MXR | SUM | MPRV | XS[1:0] | FS[1:0] | MPP[1:0] |
         --------------------------------------------------------------------------------
            1     8     1     1    1     1     1     1        2         2          2

         10     9  8     7      6      5      4      3      2     1     0
         -------------------------------------------------------------------
         | WPRI | SPP | MPIE | WPRI | SPIE | UPIE | MIE | WPRI | SIE | UIE |
         -------------------------------------------------------------------
            2      1      1     1      1      1      1     1      1     1
     */
     printf("Configuring mstatus register ...\n");

     asm("csrr t0, mstatus");  // Load MSTATUS in t0
     asm("li t1, 0xFFFFE7FF"); // Load user mode status in t1
     asm("and t0, t0, t1");    // And to change MPP bits in MSTATUS to user mode
     asm("or t0, t0, 8");      // Or to change MIE bits in MSTATUS to 1
     asm("csrw mstatus, t0");  // Write new MSTATUS

     printf("Loading user mode entry point ...\n");

     asm("la t0, user_mode_entry_point"); // Load user mode address entry point
     asm("csrw mepc, t0");                // Write mepc (machine pc) to the user entry point (used by mret)

     printf("Configuring PMP ...\n");

     /*
         PMP configuration CSRs

         31      24 23     16 15      8 7        0
         -----------------------------------------
         | pmp3cfg | pmp2cfg | pmp1cfg | pmp0cfg | pmpcfg0
         -----------------------------------------
              8         8         8         8

         31      24 23     16 15      8 7        0
         -----------------------------------------
         | pmp7cfg | pmp6cfg | pmp5cfg | pmp4cfg | pmpcfg1
         -----------------------------------------
              8         8         8         8

         31      24 23       16 15      8 7        0
         -------------------------------------------
         | pmp11cfg | pmp10cfg | pmp9cfg | pmp8cfg | pmpcfg2
         -------------------------------------------
              8          8          8         8

         31       24 23      16 15       8 7         0
         ---------------------------------------------
         | pmp15cfg | pmp14cfg | pmp13cfg | pmp12cfg | pmpcfg3
         ---------------------------------------------
              8          8          8          8

         where each configuration is

         7          6     5 4         3          2          1          0
         ---------------------------------------------------------------
         | L (WARL) | WIRI | A (WARL) | X (WARL) | W (WARL) | R (WARL) |
         ---------------------------------------------------------------
              1        2         2           1          1          1


         In pmpaddrX we store the address for which we want to configure the PMP shifted right by 2 since the pmpaddr contains bits [33:2]
         In pmpcfgX we store the configuration for that PMP
     */

     // First section covers addresses from 0 to the start of the interrupt_vector_table section
     // For this part we apply a TOR configuration with RW privileges
     asm("la t0, interrupt_vector_table"); // Load address of interrupt_vector_table
     asm("srli t0, t0, 2");                // srli of 2 
     asm("csrw pmpaddr0, t0");             // Load address in csr

     // Second section covers the addresses from the start of the interrupt_vector_table section to the start of the shadow_stack section
     // For this part we apply a TOR configuration with XRW privileges
     asm("la t0, shadow_stack"); // Load address of shadow stack
     asm("srli t0, t0, 2");      // srli of 2 
     asm("csrw pmpaddr1, t0");   // Load address in csr

     // Third section covers all the addresses from the start of the shadow_stack section to the start of the machine_setup section
     // For this part we apply a TOR configuration with RW privileges
     asm("la t0, .machine_setup"); // Load address of machine_setup
     asm("srli t0, t0, 2");        // srli of 2 
     asm("csrw pmpaddr2, t0");     // Load address in csr

     // Fourth section covers all the addresses from the start of the machine_setup section upwards
     // For this part we apply a TOR configuration with XRW privileges
     asm("li t0, 0x90000000"); // Load value big enough to cover rest of memory
     asm("srli t0, t0, 2");    // srli of 2 
     asm("csrw pmpaddr3, t0"); // Load address in csr

     /*
          Configurations:
          first:  [0 00 01 1 1 1] = 0x0F -> TOR setup (for data), not locked with XRW permissions
          second: [0 00 01 1 1 1] = 0x0F -> TOR setup (for intr_vector_table), not locked with XRW permissions
          third:  [0 00 00 0 1 1] = 0x03 -> TOR setup (for shadow_stck and cfg), not locked with RW permissions
          fourth: [0 00 01 1 1 1] = 0x0F -> TOR setup (for rest of memory), not locked with XRW permissions
     */
     asm("li t0, 0x0F030F0F"); // Load configuration bits in t0
     asm("csrw pmpcfg0, t0");  // Write configuration in csr

     printf("Jumping to user code for execution ...\n");

     asm("mret"); // Jump to user code in U mode

     return 0;
}

void terminate_execution(void)
{
     printf("Execution terminated, press Ctrl+c to close\n");
     while (1);
}
