#include <stdio.h>
#include "user_code.h"

/*
    NORMAL RUN:
        - Binary size: 5008 Byte
        - Execution time: 0.08387207984924316s

    SECURE RUN:
        - Binary size: 5152 Byte
        - Execution time: 0.08377194404602051s
        - Instrumentation time: 0.0029611587524414062s
        - Simulation time:  0s (no indirect jumps)
        - CFG extraction time: 0.0017352104187011719s
*/

int binarySearch(int arr[], int size, int target)
{
    int low = 0;
    int high = size - 1;

    while (low <= high)
    {
        int mid = low + (high - low) / 2;

        if (arr[mid] == target)
        {
            return mid; 
        }
        else if (arr[mid] < target)
        {
            low = mid + 1;
        }
        else
        {
            high = mid - 1;
        }
    }

    return -1;
}

void start_u_code()
{
    int arr[] = {71, 82, 116, 131, 143, 152, 154, 352, 389, 424, 465, 493, 536,
                 606, 714, 747, 798, 846, 850, 912, 922, 955, 988, 1022, 1058,
                 1060, 1193, 1269, 1273, 1287, 1317, 1348, 1372, 1382, 1418, 1450,
                 1493, 1504, 1511, 1521, 1540, 1562, 1569, 1573, 1595, 1619, 1635,
                 1670, 1677, 1679, 1746, 1851, 1880, 1988, 1992, 2014, 2021, 2054,
                 2060, 2116, 2192, 2207, 2269, 2281, 2331, 2459, 2499, 2536, 2668,
                 2780, 2855, 2860, 2865, 2886, 2900, 2941, 2961, 3104, 3106, 3195,
                 3271, 3298, 3307, 3389, 3505, 3569, 3623, 3629, 3633, 3663, 3729,
                 3745, 3770, 3779, 3783, 3814, 3820, 3826, 3862, 3864, 3878, 3886,
                 3887, 3907, 3955, 3961, 3977, 3994, 4009, 4058, 4064, 4155, 4163,
                 4193, 4210, 4214, 4303, 4322, 4326, 4338, 4413, 4541, 4747, 4775,
                 4852, 4879, 4881, 4906, 4915, 4958, 5096, 5162, 5181, 5231, 5329,
                 5369, 5381, 5445, 5450, 5473, 5485, 5497, 5498, 5498, 5531, 5676,
                 5688, 5716, 5717, 5723, 5997, 6031, 6046, 6077, 6178, 6229, 6229,
                 6251, 6319, 6336, 6396, 6398, 6434, 6435, 6468, 6482, 6555, 6601,
                 6615, 6688, 6703, 6749, 6833, 6836, 6880, 6931, 6953, 6979, 7007,
                 7017, 7096, 7145, 7269, 7272, 7277, 7310, 7373, 7379, 7396, 7448,
                 7453, 7476, 7556, 7567, 7601, 7648, 7657, 7743, 7744, 7863, 7876,
                 8051, 8059, 8067, 8092, 8112, 8150, 8157, 8220, 8381, 8406, 8408,
                 8441, 8443, 8502, 8563, 8569, 8570, 8608, 8673, 8677, 8731, 8748,
                 8859, 8896, 8919, 8942, 8958, 8961, 9023, 9135, 9197, 9205, 9207,
                 9212, 9224, 9230, 9568, 9569, 9622, 9804, 9895, 9919};
    int arr_size = sizeof(arr) / sizeof(arr[0]);

    int target_one = 9622;
    int target_two = 10;

    int result = binarySearch(arr, arr_size, target_one);

    if (result != -1)
    {
        printf("Element %d found at index %d\n", target_one, result);
    }
    else
    {
        printf("Element %d not found in the array\n", target_one);
    }

    result = binarySearch(arr, arr_size, target_two);

    if (result != -1)
    {
        printf("Element %d found at index %d\n", target_two, result);
    }
    else
    {
        printf("Element %d not found in the array\n", target_two);
    }
}
