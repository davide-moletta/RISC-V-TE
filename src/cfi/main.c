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
         PMP configuration

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


         in pmpaddrX we store the address for which we want to configure the PMP
         shifted right by 2 since the pmpaddr contains bits [33:2]

         in pmpcfgX we store the configuration for that PMP

     */

     // First section covers addresses from 0 to the start of the shadow stack section
     // For this part we apply a TOR configuration with XWR privileges
     asm("la t0, shadow_stack"); // Load address of shadow stack
     asm("srli t0, t0, 2");      // srli of 2 
     asm("csrw pmpaddr0, t0");   // Load address in csr

     // Second section covers the addresses of the shadow stack section
     // For this part we apply a NAPOT configuration with RW privileges

     // We take t0 which contains the address of shadow_stack shifted by 2 
     // and we apply an or with 6 to create a NAPOT space of 256B
     asm("ori t0, t0, 6");
     asm("csrw pmpaddr1, t0"); // Load address in csr

     // Third section covers all the addresses from the end of shadow stack upwards
     // For this part we apply a TOR configuration with XRW privileges
     asm("li t0, 0x90000000"); // Load value big enough to cover rest of memory
     asm("srli t0, t0, 2");    // srli of 2 
     asm("csrw pmpaddr2, t0"); // Load address in csr

     /*
          Configurations:
          first:  [0 00 01 1 1 1] = 0x0F -> TOR setup, not locked with XRW permissions
          second: [0 00 11 0 1 1] = 0x1B -> NAPOT setup, not locked with RW permissions
          third:  [0 00 01 1 1 1] = 0x0F -> TOR setup, not locked with XRW permissions
          fourth: [0 00 00 1 1 1] = 0x07 -> Disabled
     */
     asm("li t0, 0x070F1B0F"); // Load configuration bits in t0
     asm("csrw pmpcfg0, t0");  // Write configuration in csr

     /*
          OLD PMP CONFIGURATION
     */
     // asm("li t0, 0x90000000");
     // asm("srli t0, t0, 2");
     // asm("csrw pmpaddr0, t0");
     // asm("li t0, 0x0707070F");
     // asm("csrw pmpcfg0, t0");

     printf("Jumping to user code for execution ...\n");

     asm("mret"); // Jump to user code in user mode

     return 0;
}

void terminate_execution(void)
{
     printf("Execution terminated, press Ctrl+c to close\n");
     while (1);
}
