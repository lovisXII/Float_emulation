#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include "float_emu.h"

#define nbr_test 1000

// int main(){
//     srand(time(NULL)); 

//     for(int i = 0 ; i < nbr_test; i++){
//         float a = (float)rand()/((float)RAND_MAX/127);
//         float b = (float)rand()/((float)RAND_MAX/127);

//         float   reference = a * b;
//         float result = mult_float(a,b); 
        
//         if(abs(reference - result) > 0.1)
//         {
//             printf("Attended result : %f\n", reference);
//             printf("Result obtained : %f\n", result);
//             printf("\n\n");
//         }



//     }
// }

int main(){
    float a = 5.3 ;
    float b = 10 ;
    float c = a + b ;
    unsigned int a_copy = *((unsigned int*) &a);
    unsigned int b_copy = *((unsigned int*) &b);
    unsigned int c_copy = *((unsigned int*) &c);

    
    unsigned int mantice_a ;
    unsigned int mantice_b ;
    
    unsigned int exposant_a;
    unsigned int exposant_b;
    
    unsigned int sign_a;
    unsigned int sign_b;

    // | 0x800000 allow to put it in the form 1.010...
    // cause the mantice is just the part after the coma, we
    // always have to normalize with 1

    mantice_a = (a_copy & 0x7FFFFF)   ; 
    mantice_b = (b_copy & 0x7FFFFF)   ; 

 
    exposant_a = (a_copy >> 23) & 0xFF; 
    exposant_b = (b_copy >> 23) & 0xFF;

    sign_a = (a_copy >> 31) & 0x1;
    sign_b = (b_copy >> 31) & 0x1;

    printf("a is in hex:%x\n a is in dec %f", a_copy, a);
    printf(" a is :\n sign : %x\n exposant : %x\n mantice : %x\n\n", sign_a, exposant_a ,mantice_a);
    
    printf("b is in hex:%x\na is in dec %f", b_copy, b);
    
    printf(" b is :\n sign : %x\n exposant : %x\n mantice : %x\n\n", sign_b, exposant_b ,mantice_b);


    printf("result expected is :%x\n\n", c_copy);

    /* 
        The mantice design the part after the coma,
        for example if the mantice of a is 0101, it means a is :
        a = 1.010
        So we "normalise" a by doing a or with 0x800000
    */

    mantice_a |= 0x800000 ;
    mantice_b |= 0x800000 ;


    unsigned int result_exponent ;
    unsigned int mantice_a_new = mantice_a  ;
    unsigned int mantice_b_new = mantice_b ;

    
    /* 
        looking for the lowest exponent
        to shift the mantice of the number with the lowest exponent
        for instance :
        a = 1.0101*2**3
        b = 0.0011*2**4
        a has the lowest exponent, so we need to rewritte a as :
        a = 0.10101*2**4
    */
    if(sign_a == sign_b){

        printf("\ndiff exp : %d\n", (exposant_a - exposant_b) ) ;
        if((exposant_a > exposant_b)) // e_x>e_b
        {
        printf("A great \n") ;
            result_exponent = exposant_a  ;
            mantice_b_new   = mantice_b >> (exposant_a - exposant_b) ; 
        }   
        else if(exposant_b > exposant_a)
        {
        printf("B great \n") ;
            result_exponent = exposant_b ;
            mantice_a_new   = mantice_a >> (exposant_b - exposant_a) ;
        }
        else // exponent are the same
        {
            result_exponent = exposant_a ; // doesnt matter we can take any of it   
        }
        printf("mantice a after : %x\n", mantice_a_new);
        printf("mantice b after : %x\n", mantice_b_new);
        /*
        Second step is to perform the addition of the 2 mantice, the problem is that the result is on 25 bits
        and we only want 23.
        
        
        It can be on 25 bits because of the normalisation with 0x800000.

        Indeed both mantice are 23 bits so will normalization it becomes 24 and addition
        can overflow, leading to this 25 bits.

        We want to keep the msb. If the mantice has overflow it means we must reajust the exponent.
        Example :
        mantice_result = 1.0110 -> 0.10110*2
        */
        unsigned int mantice_result =  mantice_a_new + mantice_b_new ;
        
        printf("Mantice result is : %x\n",mantice_result);

        if((mantice_result >> 25 == 1))
        {
            result_exponent +=2 ;
             mantice_result = (mantice_result >> 2) & 0x7FFFFF;
        }
        else if ((mantice_result >> 24 == 1))
        {
            result_exponent ++ ;
            mantice_result = (mantice_result >> 1) & 0x7FFFFF;
        }   
        else{
            mantice_result &= 0x7FFFFF ;
        }

        

        printf(" result is :\n result_exp : %x\n result mantice : %x\n", result_exponent, mantice_result);
        int result = mantice_result | (result_exponent << 23) | (sign_a << 31);
        printf("resultat calculated is : %x\n", result);
        float result_est = *((float*) &result);
        printf("result got in float : %f\n", result_est);
    }
    else{
        printf("neg and sub nbres\n");
        printf("\ndiff exp : %d\n", exposant_a - exposant_b) ;
        
        
        unsigned int mantice_result;
        unsigned int sign_result;

        if((exposant_a > exposant_b)) // e_x>e_b
        {
            result_exponent = exposant_a  ;
            mantice_b_new   = mantice_b >> (exposant_a - exposant_b) ; 
            printf("A IS GREATER\n");
        }   
        else if(exposant_b > exposant_a)
        {
            result_exponent = exposant_b ;
            mantice_a_new   = mantice_a >> (exposant_b - exposant_a) ;
            printf("B IS GREATER\n");

        }
        else{
            printf("OPPGJEOGHZOHG GREATER\n");
            result_exponent = exposant_b ;
            
        }
        printf("mantice a after : %x\n", mantice_a_new);
        printf("mantice b after : %x\n", mantice_b_new);

        // Issue with doing the sub directly
        // The sub will change the number on 32 bits so too much 1
        // I change manually the number as negative one using Ca2

        /*
        An example will talk more :
        lets say a = 1.25 and b = -0.25

        So the mantice of a is : 01
        The mantice of b is : 0
        So we can write (i dont write the exponent value but you can calculate it) :
        a = 1.01
        b = 0.01
        a - b should be 1.0
        But this is what happend :

        a - b = a + (~b+1)
        ~b+1 = 0xFF800000
        So :
        a + b = 0x00a00000 + 0xFF800000 = 100200000
        And this is not what we want
        */

        if(sign_a) //case where a is neg, so b pos
            mantice_result = mantice_b_new + ((~mantice_a_new+1) & 0x7FFFFF);
        else // case where a is pos, so b neg
            mantice_result = mantice_a_new + ((~mantice_b_new+1) & 0x7FFFFF) ;

        printf("Mantice result is : %x\n",mantice_result );

        // Mantice result can overflow
        // So been on 24 bits, in this case we want to keep only the msb


        if((mantice_result >>24 ) == 1)
            mantice_result = mantice_result >> 1;
        else
            mantice_result &= 0x7FFFFF ;

        if(mantice_a > mantice_b && exposant_a > exposant_b )
            sign_result = sign_a ;
        else 
            sign_result = sign_b ;    
        printf(" result is :\n result_exp : %x\n result mantice : %x\n", result_exponent, mantice_result);
        int result = mantice_result | (result_exponent << 23) | (sign_result << 31);
        printf("resultat calculated is : %x\n", result);
        float result_est = *((float*) &result);
        printf("result expected in float : %f\n", result_est);
        
    }

    /*

    need to equilibrate exponent
    So must equilibrate on the smallest
    */
}
