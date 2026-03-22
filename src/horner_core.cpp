#include "horner_core.h"

data_t horner_core(
    data_t x,
    int degree,
    data_t a3,
    data_t a2,
    data_t a1,
    data_t a0)
{
#pragma HLS INLINE off
    if (degree < 0)
    {
        return static_cast<data_t>(0);
    }

    if (degree > 3)
    {
        degree = 3;
    }

    data_t coeffs[4] = {a0, a1, a2, a3};
#pragma HLS ARRAY_PARTITION variable=coeffs complete

    bool initialized = false;
    acc_t acc = 0;

    for (int d = 3; d >= 0; --d)
    {
#pragma HLS PIPELINE II=1
        if (d > degree)
        {
            continue;
        }

        data_t c = coeffs[d];
        if (!initialized)
        {
            if (c != 0)
            {
                acc = c;
                initialized = true;
            }
        }
        else
        {
            acc = acc * x;
            if (c != 0)
            {
                acc = acc + c;
            }
        }
    }

    if (!initialized)
    {
        return static_cast<data_t>(0);
    }

    return static_cast<data_t>(acc);
}
