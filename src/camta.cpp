#include "camta.h"
#include "horner_core.h"

static data_t eval_region(
    data_t x,
    int mode,
    int degree,
    data_t a3,
    data_t a2,
    data_t a1,
    data_t a0
)
{
#pragma HLS INLINE

    data_t y_horner = horner_core(x, degree, a3, a2, a1, a0);

    data_t y;

    if (mode == CAMTA_MODE_CONST)
    {
        y = a0;
    }
    else if (mode == CAMTA_MODE_ZERO)
    {
        y = static_cast<data_t>(0);
    }
    else if (mode == CAMTA_MODE_IDENTITY)
    {
        y = x;
    }
    else
    {
        y = y_horner;
    }

    return y;
}

data_t camta_unit(
    data_t x,
    data_t L_left,
    data_t L_right,
    int deg_r0,
    int deg_r1,
    int deg_r2,
    int mode_r0,
    int mode_r1,
    int mode_r2,
    data_t a3_r0, data_t a2_r0, data_t a1_r0, data_t a0_r0,
    data_t a3_r1, data_t a2_r1, data_t a1_r1, data_t a0_r1,
    data_t a3_r2, data_t a2_r2, data_t a1_r2, data_t a0_r2
)
{
#pragma HLS INLINE

    data_t a3;
    data_t a2;
    data_t a1;
    data_t a0;
    int deg;
    int mode;

    if (x < L_left)
    {
        a3 = a3_r0;
        a2 = a2_r0;
        a1 = a1_r0;
        a0 = a0_r0;
        deg = deg_r0;
        mode = mode_r0;
    }
    else if (x > L_right)
    {
        a3 = a3_r2;
        a2 = a2_r2;
        a1 = a1_r2;
        a0 = a0_r2;
        deg = deg_r2;
        mode = mode_r2;
    }
    else
    {
        a3 = a3_r1;
        a2 = a2_r1;
        a1 = a1_r1;
        a0 = a0_r1;
        deg = deg_r1;
        mode = mode_r1;
    }

    return eval_region(x, mode, deg, a3, a2, a1, a0);
}

void camta(
    const data_t x_in[],
    data_t y_out[],
    int n,
    data_t L_left,
    data_t L_right,
    int deg_r0,
    int deg_r1,
    int deg_r2,
    int mode_r0,
    int mode_r1,
    int mode_r2,
    data_t a3_r0, data_t a2_r0, data_t a1_r0, data_t a0_r0,
    data_t a3_r1, data_t a2_r1, data_t a1_r1, data_t a0_r1,
    data_t a3_r2, data_t a2_r2, data_t a1_r2, data_t a0_r2
)
{
#pragma HLS INTERFACE m_axi offset=slave port=x_in bundle=gmem0 depth=4096
#pragma HLS INTERFACE m_axi offset=slave port=y_out bundle=gmem1 depth=4096

#pragma HLS INTERFACE s_axilite port=x_in
#pragma HLS INTERFACE s_axilite port=y_out
#pragma HLS INTERFACE s_axilite port=n
#pragma HLS INTERFACE s_axilite port=L_left
#pragma HLS INTERFACE s_axilite port=L_right
#pragma HLS INTERFACE s_axilite port=deg_r0
#pragma HLS INTERFACE s_axilite port=deg_r1
#pragma HLS INTERFACE s_axilite port=deg_r2
#pragma HLS INTERFACE s_axilite port=mode_r0
#pragma HLS INTERFACE s_axilite port=mode_r1
#pragma HLS INTERFACE s_axilite port=mode_r2
#pragma HLS INTERFACE s_axilite port=a3_r0
#pragma HLS INTERFACE s_axilite port=a2_r0
#pragma HLS INTERFACE s_axilite port=a1_r0
#pragma HLS INTERFACE s_axilite port=a0_r0
#pragma HLS INTERFACE s_axilite port=a3_r1
#pragma HLS INTERFACE s_axilite port=a2_r1
#pragma HLS INTERFACE s_axilite port=a1_r1
#pragma HLS INTERFACE s_axilite port=a0_r1
#pragma HLS INTERFACE s_axilite port=a3_r2
#pragma HLS INTERFACE s_axilite port=a2_r2
#pragma HLS INTERFACE s_axilite port=a1_r2
#pragma HLS INTERFACE s_axilite port=a0_r2
#pragma HLS INTERFACE s_axilite port=return

#pragma HLS STABLE variable=L_left
#pragma HLS STABLE variable=L_right
#pragma HLS STABLE variable=deg_r0
#pragma HLS STABLE variable=deg_r1
#pragma HLS STABLE variable=deg_r2
#pragma HLS STABLE variable=mode_r0
#pragma HLS STABLE variable=mode_r1
#pragma HLS STABLE variable=mode_r2
#pragma HLS STABLE variable=a3_r0
#pragma HLS STABLE variable=a2_r0
#pragma HLS STABLE variable=a1_r0
#pragma HLS STABLE variable=a0_r0
#pragma HLS STABLE variable=a3_r1
#pragma HLS STABLE variable=a2_r1
#pragma HLS STABLE variable=a1_r1
#pragma HLS STABLE variable=a0_r1
#pragma HLS STABLE variable=a3_r2
#pragma HLS STABLE variable=a2_r2
#pragma HLS STABLE variable=a1_r2
#pragma HLS STABLE variable=a0_r2

sample_loop:
    for (int i = 0; i < n; ++i)
    {
#pragma HLS PIPELINE II=1
#pragma HLS LOOP_TRIPCOUNT min=1 max=4096 avg=1024

        data_t x = x_in[i];

        y_out[i] = camta_unit(
            x,
            L_left,
            L_right,
            deg_r0,
            deg_r1,
            deg_r2,
            mode_r0,
            mode_r1,
            mode_r2,
            a3_r0,
            a2_r0,
            a1_r0,
            a0_r0,
            a3_r1,
            a2_r1,
            a1_r1,
            a0_r1,
            a3_r2,
            a2_r2,
            a1_r2,
            a0_r2
        );
    }
}