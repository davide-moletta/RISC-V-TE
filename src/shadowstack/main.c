#include <mdk.h>
#include <intr_vector_table.h>
#include <usercode/user_entry.h>

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

    // PMP configuration
    asm("li t0, 0x90000000");
    asm("srli t0, t0, 2");
    asm("csrw pmpaddr0, t0");
    asm("li t0, 0x0707070F");
    asm("csrw pmpcfg0, t0");

    printf("Jumping to user code for execution ...\n");

    asm("mret"); // Jump to user code in user mode

    while (1);
    return 0;
}

void terminate_execution(void)
{
    printf("Execution terminated, press Ctrl+C to close\n");
    while (1);
}