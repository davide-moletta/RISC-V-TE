#include "cfg.h"
#include <stdio.h>

short check(CFG *cfg, unsigned int source, unsigned int destination) __attribute__((section(".intr_service_routines")));

// Check if the addresses match
short check(CFG *cfg, unsigned int source, unsigned int destination)
{
    for (int i = 0; i < MAX_SIZE; i++)
    {
        if (cfg->sources[i] == source && cfg->destinations[i] == destination){
            return 1;
        }
    }
    return 0;
}
