#include <stdio.h>

int main () 
{
    //initialization
    //comparaion
    //update

    for(int i = 0; i < 10; i++)
    {
        printf("%d ", i);
    }

    int ages[] = {10,11,12,13,14,15,16,17};


    //!Calculo del tamaño de un arr
    int calculatedSize = sizeof(ages) / sizeof(ages[0]);

    //? Tener cuidado con que ages sea el arreglo en el scope y no un puntero de un arreglo
    //? Este calculo del tamaño no se le puede hacer a un arreglo por referencia.

    for(int i = 0; i < calculatedSize; i++)
    {
        printf("ages[%d]: %d\n", i, ages[i]);
    }

    return 0;

}