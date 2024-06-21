#include <mdk.h>
#include <intr_vector_table.h>
#include <usercode/user_entry.h>

/*
    JTAG 
    black: ground
    red: 5V
    green: 19
    white: 18
*/

int main(void)
{
    wdt_disable(); // Disable watchdog to avoid continuous resets

    printf("Configuring interrupt vector table ...\n");

    asm("la t0, interrupt_vector_table"); // Load vector table address
    asm("ori t0, t0, 1");                 // Set MODE bit to 1 to enable vectored interrupts
    asm("csrw mtvec, t0");                // Load the address in MTVEC

    /*
        MSTATUS REGISTER (bits: [position])

        SD:     [31]                    VS:     [10:9]
        WPRI:   [30:23]                 SPP:    [8]
        TSR:    [22]                    MPIE:   [7]
        TW:     [21]                    UBE:    [6]
        TVM:    [20]                    SPIE:   [5]
        MXR:    [19]                    WPRI:   [4]
        SUM:    [18]                    MIE:    [3]
        MPRV    [17]                    WPRI:   [2]
        XS:     [16:15]                 SIE:    [1]
        FS:     [14:13]                 WPRI:   [0]
        MPP:    [12:11]
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

    printf("Execution terminated, press Ctrl+C to close\n");

    while (1);
    return 0;
}