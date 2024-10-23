#include <stdio.h>
#include "user_code.h"

void merge(int arr[], int left, int mid, int right)
{
    int i, j, k;
    int n1 = mid - left + 1;
    int n2 = right - mid;

    int L[n1], R[n2];

    for (i = 0; i < n1; i++)
        L[i] = arr[left + i];
    for (j = 0; j < n2; j++)
        R[j] = arr[mid + 1 + j];

    i = 0;
    j = 0;
    k = left;
    while (i < n1 && j < n2)
    {
        if (L[i] <= R[j])
        {
            arr[k] = L[i];
            i++;
        }
        else
        {
            arr[k] = R[j];
            j++;
        }
        k++;
    }

    while (i < n1)
    {
        arr[k] = L[i];
        i++;
        k++;
    }

    while (j < n2)
    {
        arr[k] = R[j];
        j++;
        k++;
    }
}

void mergeSort(int arr[], int left, int right)
{
    printf("Checking: %d - %d\n", left, right);
    if (left < right)
    {
        int mid = left + (right - left) / 2;

        mergeSort(arr, left, mid);
        mergeSort(arr, mid + 1, right);

        merge(arr, left, mid, right);
    }
}

void start_u_code()
{
    int arr[] = {1, 15, 10, 2, 112, 2, 14, 10, 2000, 222, 177, 24};
    int arr_size = sizeof(arr) / sizeof(arr[0]);

    printf("Start array: ");
    for (int i = 0; i < arr_size; i++)
    {
        printf("%d - ", arr[i]);
    }
    printf("\n");

    mergeSort(arr, 0, arr_size - 1);

    printf("End array: ");
    for (int i = 0; i < arr_size; i++)
    {
        printf("%d - ", arr[i]);
    }
    printf("\n");
}
