#include <stdio.h>
#include "user_code.h"

/*
    DUFF'S DEVICE
    
    NORMAL RUN:
        - Binary size: 3904 Byte
        - Execution time: 0.09779596328735352s

    SECURE RUN:
        - Binary size: 4336 Byte
        - Execution time: 0.09779952598571777s
        - Instrumentation time: 0.002654552459716797s
        - Simulation time: 0s (no indirect jumps)
        - CFG extraction time: 0.0016314983367919922s
*/


void duff_copy(char *to, const char *from, size_t count)
{
    size_t n = (count + 7) / 8;

    switch (count % 8)
    {
    case 0:
        do
        {
            *to++ = *from++;
        /* fallthrough */
        case 7:
            *to++ = *from++;
        /* fallthrough */
        case 6:
            *to++ = *from++;
        /* fallthrough */
        case 5:
            *to++ = *from++;
        /* fallthrough */
        case 4:
            *to++ = *from++;
        /* fallthrough */
        case 3:
            *to++ = *from++;
        /* fallthrough */
        case 2:
            *to++ = *from++;
        /* fallthrough */
        case 1:
            *to++ = *from++;
        } while (--n > 0);
    }
}

void start_u_code()
{
    char source[] = "This is a very long source string that needs to be copied in the destination array using Duff's device!";
    char destination[105];

    duff_copy(destination, source, sizeof(source));

    printf("Source: %s\n", source);
    printf("Destination: %s\n", destination);
}
