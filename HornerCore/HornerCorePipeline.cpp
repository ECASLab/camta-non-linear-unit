#include <ap_fixed.h>

typedef ap_fixed<16,6> data_t;

data_t horner_core_pipeline(
    data_t x,
    data_t a3,
    data_t a2,
    data_t a1,
    data_t a0)
{
#pragma HLS PIPELINE II=1
#pragma HLS INLINE off

    data_t acc0, acc1, acc2;

    acc0 = a3 * x + a2;
    acc1 = acc0 * x + a1;
    acc2 = acc1 * x + a0;

    return acc2;
}