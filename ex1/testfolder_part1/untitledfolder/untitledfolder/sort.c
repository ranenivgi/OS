#include <stdio.h>
#include <stdlib.h>
/* word */
int compare(const void *a, const void *b) {
    return (*(int*)a - *(int*)b);
}

void printArray(int arr[], int size) {
    int i;
    for (i = 0; i < size; i++)
        printf("%d ", arr[i]);
    printf("\n");
}

int main() {
    int arr[] = {10, 7, 8, 9, 1, 5};
    int n = sizeof(arr) / sizeof(arr[0]);
    
    printf("Array before sorting: ");
    printArray(arr, n);
    
    qsort(arr, n, sizeof(int), compare);
    
    printf("Array after sorting: ");
    printArray(arr, n);
    
    return 0;
}