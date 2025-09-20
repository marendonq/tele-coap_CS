#include <stdio.h>

typedef struct
{
    int lenght;
    int width;

} rectangle;

typedef struct
{
    int x;
    int y;

} position;

typedef struct 
{
    char owner[30];
    rectangle rectangle;
    position position;


}buildingPlan;

int main(){

    rectangle myRectangle = {5,10};
    printf("Length: %d. Width: %d\n", myRectangle.lenght, myRectangle.width);

    buildingPlan myHouse = {"Samuel Madrid", {10,11}, {32,48}};
    
    int size = 3;

    position path[] = {{0,1},{0,2},{0,1}};

    for(int i = 0; i < size; i++)
    {
        printf("%d %d\n", path[i].x, path[i].y);
    }

    //! Pointer to a struct

    buildingPlan *structPointer = &myHouse;

    printf("Position x: %d\n",structPointer->position.x);

    return 0;
}