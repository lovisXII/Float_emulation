#include <stdio.h>
#include <stdlib.h>
#include <time.h>
// #define DEBUG
// Will only word for addition for now

float addition(float op1_float, float op2_float){

    unsigned int op1 = *((unsigned int*) &op1_float);
    unsigned int op2 = *((unsigned int*) &op2_float);
    // s[31] exp[30:23] mantissa[22:0]
    unsigned int mask_exposant = 0xFF;
    unsigned int mask_mantissa = 0x7FFFFF;

    unsigned int op1_sign     = (op1 >> 31);
    unsigned int op1_exposant = (op1 >> 23) & mask_exposant;
    unsigned int op1_mantissa = (op1        & mask_mantissa) << 1; // shift 1 to add the guard bit
    #ifdef DEBUG
        printf("Op1 is :\n s : %x, e : %x, m : %x\n", op1_sign, op1_exposant ,op1_mantissa);
    #endif
    unsigned int op2_sign     = (op2 >> 31);
    unsigned int op2_exposant = (op2 >> 23) & mask_exposant;
    unsigned int op2_mantissa = (op2        & mask_mantissa) << 1; //shift 1 to add the guard bit
    #ifdef DEBUG
    printf("op2 is :\n s : %x, e : %x, m : %x\n", op2_sign, op2_exposant ,op2_mantissa);
    #endif
    // Mantissa is on 24 bits, 23 bits of the ieee 754 mantissa and one guard bit
    // Need to add the hidden bit, will be usefull if normalization is performed
    op1_mantissa = (0b1 << 24) | op1_mantissa;
    op2_mantissa = (0b1 << 24) | op2_mantissa;

    // Exponent difference
    unsigned int d = op1_exposant + ~op2_exposant + 1;
    #ifdef DEBUG
    printf("exponent diff : %d\n", d);
    #endif

    // Sign may be negative
    unsigned int d_neg   = (d >> 31) ? 1 : 0 ;
    unsigned int d_zero  = d == 0;
    unsigned int exp_res;

    if     (d_neg)              exp_res = op2_exposant;
    else if(~d_neg & ~d_zero)   exp_res = op1_exposant;
    else                        exp_res = op1_exposant; // both are equals, doesn't matter which one is taken

    // Going to perform alignement
    // Several cases can occur :
    // * d < 0  : d_neg = 1              ; op1 - op2 < 0 => op1 < op2
    // * d > 0  : d_neg = 0 & d_zero = 0 ; op1 - op2 > 0 => op1 > op2
    // * d == 0 : d_zero = 0
    // Always the smallest exponent aligned on the bigger one
    // (~d + 1) : if difference of operand if < 0 must transform operand to > 0

    unsigned int shift_value = (d_neg) ? (~d + 1) : d;
    #ifdef DEBUG
    printf(" Shift value : %u \n", shift_value);
    printf(" d is negative : %d\n", d_neg);
    #endif
    if      (d_neg)   op1_mantissa = op1_mantissa >> (~d + 1);
    else if (~d_zero) op2_mantissa = op2_mantissa >>   d;

    #ifdef DEBUG
    printf(" Mantissa op1 after normalization : %x \n", op1_mantissa);
    printf(" Mantissa op2 after normalization : %x \n", op2_mantissa);
    #endif
    unsigned int mantisse_res = op1_mantissa + op2_mantissa;
    #ifdef DEBUG
    printf(" Mantissa before overflow gestion : %x \n", mantisse_res);
    #endif

    // Need to detect mantissa overflow
    // bit [0]    : round bit
    // bit [23:1] : mantissa
    // bit [24]   : normalized bit
    // bit [25]   : overflow bit
    unsigned int overflow     = mantisse_res >> 25;
    // If overflow is detected, need to shift right the mantissa
    // and increment the exponent
    mantisse_res = (overflow) ? (mantisse_res >> 1) : mantisse_res;
    exp_res      = (overflow) ? exp_res + 1         : exp_res;
    #ifdef DEBUG
    printf(" Mantissa after overflow gestion  : %x \n", mantisse_res);
    #endif
    // Then need to perform the rounding
    // We'll use rounding to nearest
    unsigned int l_bit     = (mantisse_res & 0b10) >> 1;
    unsigned int guard_bit =  mantisse_res & 0b1;
    unsigned int round     = guard_bit & guard_bit;
    #ifdef DEBUG
    printf(" Round is : %d \n", round);
    #endif
    mantisse_res           = mantisse_res + round;
    // Sign result
    // If sign is the same result sign is the same
    // If it's not the same it will depend of d
    unsigned int sign_res = 0b0;
    // Result
    // mantisse_res >> 1 : Right shift to remove the hidden bit
    unsigned int res_unsigned = (sign_res << 31) | (exp_res << 23) | ((mantisse_res >> 1) & mask_mantissa);
    #ifdef DEBUG
    printf("Sign res is : %x\n"    , sign_res);
    printf("Exponent is : %x\n"    , exp_res);
    printf("Mantissa res is : %x\n", mantisse_res & mask_mantissa);
    printf("Result is : %x\n", res_unsigned);
    #endif
    float res = *(float*) &res_unsigned;
    #ifdef DEBUG
    printf("Result is : %f\n", res);
    #endif
    return res;
}

int main(){
    // Seed the random number generator with the current time
    srand(time(NULL));


    // To generate a random float within a specific range (e.g., between 2.0 and 5.0)
    float minValue = 0.0;
    float maxValue = 10000.0;
    int i;
    for(i = 0; i < 100; i++){
        float randomRange = (float)rand() / RAND_MAX; // Random number between 0 and 1
        float number1 = minValue + randomRange * (maxValue - minValue);
        float number2 = minValue + randomRange * (maxValue - minValue);
        float addition_ref = number1 + number2;
        if (addition_ref - addition(number1, number2) > 0,1)
        {
            printf("Error, value calculated : %f\n", addition(number1, number2));
            printf("Value expected : %f\n", addition_ref);
        }

    }
}
