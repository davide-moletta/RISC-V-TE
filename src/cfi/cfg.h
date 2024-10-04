#ifndef CFG_H
#define CFG_H

#define MAX_SIZE 63

typedef struct
{
    unsigned int sources[MAX_SIZE];
    unsigned int destinations[MAX_SIZE];
} CFG;

short check(CFG *cfg, unsigned int source, unsigned int destination) __attribute__((section(".intr_service_routines")));

#endif
