#include <stdio.h>
#include <stdbool.h>

int main()
{

    //!Ternary operators
    bool hasMoney;
    int balance = 0;

    hasMoney = balance > 0 ? 1 : 0;

    hasMoney ? printf("Congrats\n") : printf("Sorry fella\n");

    return 0;
}