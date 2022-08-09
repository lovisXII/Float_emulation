
#include <stdio.h>

void int2float(int* a, float * result){
    void * a_copy ;
    a_copy = &a;
    result = (float*) a ;
}

float mult_float(float a, float b){
    
    unsigned int a_copy = *((unsigned int*) &a) ;
    unsigned int b_copy = *((unsigned int*) &b) ;


    
    unsigned long int mantice_a ;
    unsigned long int mantice_b ;
    
    int exposant_a;
    int exposant_b;
    
    int sign_a;
    int sign_b;

    mantice_a = (a_copy & 0x7FFFFF)  ; 
    mantice_b = (b_copy & 0x7FFFFF)  ; 
 
    

    exposant_a = (a_copy >> 23) & 0xFF; 
    exposant_b = (b_copy >> 23) & 0xFF;

    sign_a = (a_copy >> 31) & 0x1;
    sign_b = (b_copy >> 31) & 0x1;


    // Mantice calculation :

    // Getting the result of the mult, 48 bits results

    unsigned long int  result_mantice = (mantice_a  | 0x800000) * (mantice_b | 0x800000) ;
    /*
    We need to keep only the msb of the result, only these bits have meaning
    We're using unsigned long int so 64 bits 

    So we need to get rid of the bits from 64 to 48,
    this is why we need to shift left the result by 16.
    Like this our result will be in the bits 64 to 17.
    */

    result_mantice = (result_mantice << 16) ;
    unsigned long int result_mantice_copy = result_mantice;

    /*
    We need to remove the msb 1, so to do this we use a counter.
    While the msb are 0 we increment cpt. When the bit is 1 we stop 
    incrementing it

    We do this to normalize the mantice.
    The normalise value is the value of cpt - 1 . 
    */

   int cpt = 0 ;
    while((result_mantice_copy >> 63) != 1)
    {
        cpt ++;
        result_mantice_copy = result_mantice_copy << 1;
    } 

    // We get the mantice value after normalization

    result_mantice = (result_mantice << (cpt + 1)) >> 41 ;

    int result_exposant = (exposant_a + exposant_b) - 127 + (1-cpt) ;
   
    // Sign calculation

    int result_sign = sign_a ^ sign_b ;


    int result_copy = result_mantice | (result_exposant << 23) | (result_sign << 31);

   
    float result_flaot = *((float*)&result_copy);;
    return  result_flaot;
}

float add_float(float a, float b){
    unsigned int a_copy = *((unsigned int*) &a);
    unsigned int b_copy = *((unsigned int*) &b);

    
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

    // printf("a is in hex:%x\n a is in dec %f", a_copy, a);
    // printf(" a is :\n sign : %x\n exposant : %x\n mantice : %x\n\n", sign_a, exposant_a ,mantice_a);
    
    // printf("b is in hex:%x\na is in dec %f", b_copy, b);




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

        if((exposant_a > exposant_b)) // e_x>e_b
        {
            result_exponent = exposant_a  ;
            mantice_b_new   = mantice_b >> (exposant_a - exposant_b) ; 
        }   
        else if(exposant_b > exposant_a)
        {
            result_exponent = exposant_b ;
            mantice_a_new   = mantice_a >> (exposant_b - exposant_a) ;
        }
        else // exponent are the same
        {
            result_exponent = exposant_a ; // doesnt matter we can take any of it   
        }
        // printf("mantice a after : %x\n", mantice_a_new);
        // printf("mantice b after : %x\n", mantice_b_new);
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
        
        // printf("Mantice result is : %x\n",mantice_result);

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

        

        // printf(" result is :\n result_exp : %x\n result mantice : %x\n", result_exponent, mantice_result);
        int result = mantice_result | (result_exponent << 23) | (sign_a << 31);
        // printf("resultat calculated is : %x\n", result);
        float result_est = *((float*) &result);
        return result_est;
        // printf("result got in float : %f\n", result_est);
    }
}