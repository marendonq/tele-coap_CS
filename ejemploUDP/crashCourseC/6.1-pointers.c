#include <stdio.h>

void square(int *a)
{
    *a *= *a;
}


int main()
{

    int a = 100;
    int *b = &a;

    printf("a = %d\n", a);
    printf("*b = %d\n", *b);
    
    *b = 200;
    
    printf("a = %d\n", a);
    printf("*b = %d\n", *b);
    
    a = 300;
    
    printf("a = %d\n", a);
    printf("*b = %d\n", *b);
    
    int c = 6000;
    
    b = &c;
    
    printf("a = %d\n", a);
    printf("*b = %d\n", *b);
    
    square(b);
    
    printf("c = %d\n", c);
    printf("*b = %d\n", *b);



    return 0;

}