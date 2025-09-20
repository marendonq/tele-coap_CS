#include "5.2-libraries.h"
#include <stdio.h>


int main()
{
    int size = 6;

    int arr[] = {12,22,31,43,5333,36};

    int gts = gtsValue(arr, size);

    printf("The greatest number in arr is: %d\n",gts);

}