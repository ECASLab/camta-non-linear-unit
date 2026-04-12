#include "camta.h"
#include "horner_core.h"

void camta(
    const word_t x_in[],
    word_t y_out[],
    int n,
    data_t L,
    int deg_r0,
    int deg_r1,
    int deg_r2,
    data_t a3_r0, data_t a2_r0, data_t a1_r0, data_t a0_r0,
    data_t a3_r1, data_t a2_r1, data_t a1_r1, data_t a0_r1,
    data_t a3_r2, data_t a2_r2, data_t a1_r2, data_t a0_r2
)
{
#pragma HLS INTERFACE m_axi offset=slave port=x_in bundle=gmem0 depth=1024
#pragma HLS INTERFACE m_axi offset=slave port=y_out bundle=gmem1 depth=1024
#pragma HLS INTERFACE s_axilite port=x_in
#pragma HLS INTERFACE s_axilite port=y_out
#pragma HLS INTERFACE s_axilite port=n
#pragma HLS INTERFACE s_axilite port=L
#pragma HLS INTERFACE s_axilite port=deg_r0
#pragma HLS INTERFACE s_axilite port=deg_r1
#pragma HLS INTERFACE s_axilite port=deg_r2
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

#pragma HLS STABLE variable=L
#pragma HLS STABLE variable=deg_r0
#pragma HLS STABLE variable=deg_r1
#pragma HLS STABLE variable=deg_r2
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

    const int n_words = (n + CAMTA_LANES - 1) / CAMTA_LANES;

word_loop:
    for (int w = 0; w < n_words; ++w)
    {
#pragma HLS PIPELINE II=1
#pragma HLS LOOP_TRIPCOUNT min=1 max=1024 avg=128
        word_t in_word = x_in[w];
        word_t out_word = 0;

    lane_loop:
        for (int lane = 0; lane < CAMTA_LANES; ++lane)
        {
#pragma HLS UNROLL
            const int idx = w * CAMTA_LANES + lane;
            data_t y = 0;

            if (idx < n)
            {
                data_t x = camta_unpack_lane(in_word, lane);
                data_t a3;
                data_t a2;
                data_t a1;
                data_t a0;
                int deg;

                if (x < -L)
                {
                    a3 = a3_r0; a2 = a2_r0; a1 = a1_r0; a0 = a0_r0;
                    deg = deg_r0;
                }
                else if (x > L)
                {
                    a3 = a3_r2; a2 = a2_r2; a1 = a1_r2; a0 = a0_r2;
                    deg = deg_r2;
                }
                else
                {
                    a3 = a3_r1; a2 = a2_r1; a1 = a1_r1; a0 = a0_r1;
                    deg = deg_r1;
                }

                y = horner_core(x, deg, a3, a2, a1, a0);
            }

            camta_pack_lane(out_word, lane, y);
        }

        y_out[w] = out_word;
    }
}
