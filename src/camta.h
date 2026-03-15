#ifndef CAMTA_H
#define CAMTA_H
 
#include "ap_fixed.h"
 
typedef ap_fixed<16,6> data_t;
 
data_t camta(
    data_t x,
    data_t L,
 
    data_t a3_r0, data_t a2_r0, data_t a1_r0, data_t a0_r0,
    data_t a3_r1, data_t a2_r1, data_t a1_r1, data_t a0_r1,
    data_t a3_r2, data_t a2_r2, data_t a1_r2, data_t a0_r2
);
 
#endif