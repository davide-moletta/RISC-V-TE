#include <mdk.h>
#include "intr_vector_table.h"
#include "usercode/user_entry.h"
#include "shadow_stack.h"

int main(void)                 __attribute__((section(".machine_setup")));
void terminate_execution(void) __attribute__((section(".machine_setup")));
/*
    JTAG
    black: ground
    red: 5V
    green: 19
    white: 18
*/

// openocd -f openocd/esp32c3-builtin.cfg
// riscv32-esp-elf-gdb -x .gdbinit src/shadowstack/firmware.elf

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
     asm("or t0, t0, 8");      // Or to change MIE bits in MSTATUS to 0
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
         shifted right by 2 since the pmpadrr contains bits [33:2]

         in pmpcfgX we store the configuration for that PMP

     */

     // asm("la t0, shadow_stack");
     // unsigned long shads;
     // asm volatile("add %0, t0, x0" : "=r"(shads));
     // printf("shadow stack address is: %8lx\n", shads);

     // get the address of shadowstack section
     // get the address of interrupt vector table
     // insert shadowstack address as TOR with R-W privileges
     // insert intr vector table as NAPOT

     asm("li t0, 0x90000000");
     asm("srli t0, t0, 2");
     asm("csrw pmpaddr0, t0");
     asm("li t0, 0x0707070F"); // 00000111 00000111 00000111 00001111
     asm("csrw pmpcfg0, t0");

     printf("Jumping to user code for execution ...\n");

     asm("mret"); // Jump to user code in user mode

     return 0;
}

void terminate_execution(void)
{
     printf("Execution terminated, press Ctrl+C to close\n");
     while (1);
}