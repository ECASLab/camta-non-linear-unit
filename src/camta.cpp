#include "camta.h"
#include "horner_core.h"
 
data_t camta(
    data_t x,
    data_t L,
 
    data_t a3_r0, data_t a2_r0, data_t a1_r0, data_t a0_r0,
    data_t a3_r1, data_t a2_r1, data_t a1_r1, data_t a0_r1,
    data_t a3_r2, data_t a2_r2, data_t a1_r2, data_t a0_r2
){
 
#pragma HLS PIPELINE II=1
 
    data_t a3,a2,a1,a0;
 
    if(x < -L){
        a3=a3_r0; a2=a2_r0; a1=a1_r0; a0=a0_r0;
    }
    else if(x > L){
        a3=a3_r2; a2=a2_r2; a1=a1_r2; a0=a0_r2;
    }
    else{
        a3=a3_r1; a2=a2_r1; a1=a1_r1; a0=a0_r1;
    }
 
    return horner_core(x,a3,a2,a1,a0);
}