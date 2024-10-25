#include <stdio.h>
#include <stdint.h>
#include "user_code.h"

/*
    CRC

    NORMAL RUN:
        - Binary size: 3616 Byte
        - Execution time: 0.08025240898132324s

    SECURE RUN:
        - Binary size: 4032 Byte
        - Execution time: 0.08026152320861816s
        - Instrumentation time: 0.002025127410888672s
        - Simulation time: 0s (no indirect jumps)
        - CFG extraction time: 0.0015430450439453125s
*/

#define CRC32_POLYNOMIAL 0xEDB88320

uint32_t crc32(uint8_t *data, size_t length)
{
    uint32_t crc = 0xFFFFFFFF;

    for (size_t i = 0; i < length; i++)
    {
        crc ^= data[i];

        for (uint8_t j = 0; j < 8; j++)
        {
            if (crc & 1)
            {
                crc = (crc >> 1) ^ CRC32_POLYNOMIAL;
            }
            else
            {
                crc = crc >> 1;
            }
        }
    }

    return ~crc;
}

void start_u_code()
{
    uint8_t data[] = "Hello, World!";
    size_t length = sizeof(data) - 1;

    uint32_t crc = crc32(data, length);

    printf("CRC-32/ISO-HDLC: %08lx\n", (unsigned long)crc);
}
