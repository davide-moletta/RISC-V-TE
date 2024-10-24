#include "cfg.h"
#include <stdio.h>

int check(unsigned int source, unsigned int destination) __attribute__((section(".intr_service_routines")));
void* bin_search(const void* key, const void* base, size_t num, size_t size, int (*cmp)(const void*, const void*)) __attribute__((section(".intr_service_routines")));
int compare(const void* A, const void* B) __attribute__((section(".intr_service_routines")));

// Definition of control flow graph and its size, do not modify as the instrumenter will take care of this
__attribute__((section(".cfg"))) unsigned int cfg[][2] = {{1,1}};
size_t cfg_size = 2;

// Comparison function for binary search
int compare(const void* A, const void* B) {
    const int* pair1 = (const int*)A;
    const int* pair2 = (const int*)B;

    // For each pair compare the sources, if they are the same, compare the destinations
    for (int i = 0; i < 2; ++i) {
        if (pair1[i] < pair2[i]) return -1;
        if (pair1[i] > pair2[i]) return 1;
    }

    // If both source and destination are equal return 0
    return 0;
}

// Binary search function
void* bin_search(const void* key, const void* base, size_t num, size_t size, int (*cmp)(const void*, const void*)) { 
    size_t low = 0;
    size_t high = num - 1;

    while (low <= high) {
        size_t mid = low + (high - low) / 2;    // Calculate mid index
        const void* mid_element = (const char*)base + mid * size;

        int cmp_result = cmp(key, mid_element); // Use renamed compare for comparison
        if (cmp_result < 0) {
            high = mid - 1;                     // Move left
        } else if (cmp_result > 0) {
            low = mid + 1;                      // Move right
        } else {
            return (void*)mid_element;          // Found the element
        }
    }

    return NULL;                                // Element not found
}

// Check if the source and destination addresses match
int check(unsigned int source, unsigned int destination)
{
    unsigned int target[2] = {source, destination};

    // Perform binary search
    int* result = (int*)bin_search(target, cfg, cfg_size, sizeof(cfg[0]), compare);

    // Return 1 if pair found, 0 if not
    return result != NULL;
}
