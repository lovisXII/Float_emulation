#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include "test.h"

#define nbr_test 100

int main(){
    srand(time(NULL)); 

    for(int i = 0 ; i < nbr_test; i++){
        float a = (float)rand()/((float)RAND_MAX/127);
        float b = (float)rand()/((float)RAND_MAX/127);

        float   reference = a * b;
        float* result = mult_float(&a,&b); 
        
        if((a*b) != *result)
        {
            printf("Attended result : %f\n", reference);
            printf("Result obtained : %f\n", *result);
        }



    }
}