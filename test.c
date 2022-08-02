
#include <stdio.h>

// __asm__(".section .text") ;
// __asm__(".global _start") ;

// __asm__("_start:");
// __asm__("addi x1,x1, 4");
// __asm__("sub x2, x2,x1 ");
// __asm__("jal x5, main");


float* int2float(int a){
    void * a_copy ;
    a_copy = &a;
    float * result = (float*) a_copy ;
    return result;
}

float* mult_float(void *a, void* b){
    
    unsigned int a_copy = *(unsigned int*) a ;
    unsigned int b_copy = *(unsigned int*) b ;

    printf("printing a : %x\n",a_copy);
    printf("printing b : %x\n",b_copy);


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

    printf("exposant a : %x\n",exposant_a);
    printf("exposant b : %x\n\n",exposant_b);

    printf("sign a : %x\n",sign_a);
    printf("sign b: %x\n\n",sign_b);
   
    printf("mantice a : %lx\n",mantice_a);
    printf("mantice b : %lx\n\n",mantice_b);

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
        printf("mantice before while : %lx\n", result_mantice_copy) ;
    while((result_mantice_copy >> 63) != 1)
    {
        printf("mantice before shift : %lx\n", result_mantice_copy) ;
        cpt ++;
        result_mantice_copy = result_mantice_copy << 1;
        printf("mantice after shift : %lx\n", result_mantice_copy) ;
    } 
    printf("cpt : %d\n", cpt);

    // We get the mantice value after normalization

    result_mantice = (result_mantice << (cpt + 1)) >> 41 ;

    int result_exposant = (exposant_a + exposant_b) - 127 + (1-cpt) ;
   
    // Sign calculation

    int result_sign = sign_a ^ sign_b ;


 
    printf("result mantice : %lx\n", result_mantice);
    printf("result exposant  : %x\n",result_exposant);
    printf("result sign      : %x\n",result_sign);

    int result_copy = result_mantice | (result_exposant << 23) | (result_sign << 31);

    printf("result : %x\n",result_copy);
    return int2float(result_copy) ;
}




int main(){


    float a = 5.25 ;
    float b = 2.25;
    float* result ;
    float comparaison = 27.5625 ;


    result = mult_float(&a,&b);

    printf("calcul : %f\n", *result);

}
