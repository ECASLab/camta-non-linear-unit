#include "horner_core.h"

data_t horner_core(
    data_t x,
    data_t a3,
    data_t a2,
    data_t a1,
    data_t a0)
{
#pragma HLS INLINE off
    acc_t acc = a3;
    acc_t mul;
#pragma HLS RESOURCE variable=mul core=Mul_LUT
    mul = acc * x;
    acc = mul + a2;
    mul = acc * x;
    acc = mul + a1;
    mul = acc * x;
    acc = mul + a0;
    return (data_t)acc;
}