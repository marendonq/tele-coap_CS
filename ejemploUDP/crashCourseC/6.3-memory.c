#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

// La variable existe en toda la ejecucion del programa y el scope es el mismo
// int static x = 100;

typedef struct 
{
    char name[30];
    int age;
    bool isVerified;

} user;

user *createUser(char name[], int age, bool isVerified)
{
    user *newUser = malloc(sizeof(user));
    strcpy(newUser->name, name);
    newUser->age = age;
    newUser->isVerified = isVerified;
    return newUser;
};

int main()
{
    int size;
    printf("How many int elements u nedd bro?: ");
    scanf("%d", &size);

    int *arr = malloc(size * sizeof(int));

    if (arr == 0)
    {
        printf("Invalid pointer. Erro allocating memory\n");
    }
    else
    {
        printf("You're goo to go\n");
    }

    for (int i = 0; i < size; i++)
    {
        scanf("%d", &arr[i]);
    }

    printf("Your array:\n");

    for (int i = 0; i < size; i++)
    {
        printf("%d ", arr[i]);
    }

    printf("\n");

    free(arr);

    user *me = createUser("Samuel Madrid", 20, true);

    printf("%d\n", me->age);
    free(me);

    return 0;
}