#include <ap_fixed.h>

typedef ap_fixed<16,6> data_t;
typedef ap_fixed<32,12> acc_t;

ap_fixed<16,6> horner_core(
    ap_fixed<16,6> x,
    ap_fixed<16,6> a3,
    ap_fixed<16,6> a2,
    ap_fixed<16,6> a1,
    ap_fixed<16,6> a0)
{
#pragma HLS INLINE off
#pragma HLS PIPELINE off
#pragma HLS ALLOCATION instances=mul limit=1 operation
#pragma HLS ALLOCATION instances=add limit=1 operation

    acc_t acc = a3;

    acc = acc * x + a2;
    acc = acc * x + a1;
    acc = acc * x + a0;

    return (data_t)acc;
}
