#include <stdlib.h>
#include <stdio.h>

int sum(int x, int y){
    return (x+y)*2;
}

void main(){
    int x = 1;
    int y = 2;
    int result;

    result = sum(x,y);
    printf("sum %i\n", result);
    return;
}