#include <stdlib.h>
#include <stdio.h>



int main(){
    void * vec1 = (double *) malloc(sizeof(double) * 2);
    void * vec2;

    ((double *) vec1)[0] = 4;
    ((double *) vec1)[1]= 3;


    vec2 = &(((double *) vec1)[1]);
    printf("0: %.2f\n1: %.2f\n",((double *) vec1)[0],((double *) vec1)[1]);
    printf("vec2 %.2f\n", ((double *) vec2));

    return 0;
}