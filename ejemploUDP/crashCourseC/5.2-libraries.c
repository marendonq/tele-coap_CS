#include "5.2-libraries.h"
int gtsValue(int arr[], int size)
{

    int gts = arr[0];
    for(int i = 0; i < size; i++)
    {
        if(arr[i] > gts)
        {
            gts = arr[i];
        }
    }

    return gts;
}