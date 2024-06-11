#include <mdk.h>

// int main(void) {
//   wdt_disable();
//   int a = 2 +2;
//   printf("a: %d", a);

//   return 0;
// }

__attribute__((section(".prova"))) void exception_handler(){
    printf("Exception");
    while (1)
    {
        /* code */
    }
    

    asm("nop");
}

void trap_base() {
    printf("exception");
    asm("j exception_handler");
}

void user_mode_entry_point(){ //0x42007d38
    asm("la t0, 0x0"); 
    asm("csrw mie, t0"); 
    printf("Hello from user mode");
    while(1);
    asm("nop");
   // return 0;
}

/*MSTATUS REGISTER

31 30-23 22  21 20  19  18  17   16-15   14-13   12-11    10-9    8   7    6   5    4    3   2    1   0
SD WPRI  TSR TW TVM MXR SUM MPRV XS[1:0] FS[1:0] MPP[1:0] VS[1:0] SPP MPIE UBE SPIE WPRI MIE WPRI SIE WPRI

*/

int main(void)
{
    wdt_disable();

    asm("la t0, trap_base");
    asm("csrw mtvec, t0");
    /*
        Load mstatus in t0
        Load user mode status in t1
        And to change MPP bits in mstatus to user mode
        Or to change MIE bits in mstatus to 0
        Write new mstatus
    */
    asm("csrr t0, mstatus"); 
    asm("li t1, 0xFFFFE7FF"); //    11111111111111111110011111111111
    asm("and t0, t0, t1");
    asm("or t0, t0, 8"); //    0000000000000000000000000001000
    asm("csrw mstatus, t0");
    printf("Hello world from the ESP board!");

    /*
        Load user mode address entry point
        Write mepc (machine pc) to the user entry point (used by mret)
    */
    asm("la t0, user_mode_entry_point");
    asm("csrw mepc, t0"); 


    asm("li t0, 0x90000000");
    asm("srli t0, t0, 2");
    asm("csrw pmpaddr0, t0");
    asm("li t0, 0x0707070F");
    asm("csrw pmpcfg0, t0");

    asm("mret");
    printf("Hello world from the ESP board!");
    while (1);
    
    return 0;
}

// int main(void){

//     asm("la t0, trap_base");
//     asm("csrw mtvec, t0");
//     asm("j app_entry");

//     delay_ms(10000);
//     return 0;
// }
