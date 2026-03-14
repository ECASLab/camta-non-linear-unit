#include <ap_fixed.h>

typedef ap_fixed<16,6> data_t;

ap_fixed<16,6> horner_core(
    ap_fixed<16,6> x,
    ap_fixed<16,6> a3,
    ap_fixed<16,6> a2,
    ap_fixed<16,6> a1,
    ap_fixed<16,6> a0)
{
#pragma HLS PIPELINE off

    ap_fixed<16,6> acc;

    acc = a3 * x + a2;
    acc = acc * x + a1;
    acc = acc * x + a0;

    return acc;
}