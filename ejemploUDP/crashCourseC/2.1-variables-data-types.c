#include <stdio.h>
#include <stdbool.h>

int main()
{
    int a = 10;
    double b = 10.5;
    printf("%lu\n", sizeof(b)*8);
    float c = 10.5;
    printf("%lu\n", sizeof(c))*8;
    char d = 'a';
    char e[] = "Char array";
    bool f = false;


    //! Implicit conversion type
    int zero = .99999999;
    printf("%d\n", zero);
    

    int slices = 17;
    int persons = 2;
    //!Explicit conversion type or casting
    double slicesPerPerson = (double)slices / persons;

    printf("%lf\n", slicesPerPerson);

    double var1 = 25 / 2 * 2; //24
    double var2 = 25 / 2 * 2.0; //24
    double var3 = 25.0 / 2 * 2; //25
    double var4 = (double) 25 / 2 * 2; //25

    printf("%f\n", var1);
    printf("%f\n", var2);
    printf("%f\n", var3);
    printf("%f\n", var4);

    return 0;
}