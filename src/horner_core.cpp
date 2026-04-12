#include "horner_core.h"

data_t horner_core(
    data_t x,
    int degree,
    data_t a3,
    data_t a2,
    data_t a1,
    data_t a0)
{
#pragma HLS INLINE

    int deg = degree;
    if (deg < 0)
    {
        return static_cast<data_t>(0);
    }

    if (deg > 3)
    {
        deg = 3;
    }

    data_t coeffs[4] = {a0, a1, a2, a3};
#pragma HLS ARRAY_PARTITION variable=coeffs complete

    acc_t acc = 0;

    for (int d = 3; d >= 0; --d)
    {
#pragma HLS UNROLL
        if (d <= deg)
        {
            acc = acc * x + coeffs[d];
        }
    }

    return static_cast<data_t>(acc);
}
