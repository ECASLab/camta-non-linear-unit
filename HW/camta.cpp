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
) {
#pragma HLS INLINE
    data_t y_horner = horner_core(x, degree, a3, a2, a1, a0);
    data_t y;
    if (mode == CAMTA_MODE_CONST) {
        y = a0;
    }
    else if (mode == CAMTA_MODE_ZERO) {
        y = static_cast<data_t>(0);
    }
    else if (mode == CAMTA_MODE_IDENTITY) {
        y = x;
    }
    else {
        y = y_horner;
    }
    return y;
}
static bool uses_abs_input(int sym_mode) {
#pragma HLS INLINE
    return (sym_mode == CAMTA_SYM_ODD)        ||
           (sym_mode == CAMTA_SYM_COMPLEMENT) ||
           (sym_mode == CAMTA_SYM_RESIDUAL)   ||
           (sym_mode == CAMTA_SYM_EVEN);
}
static data_t apply_symmetry_post(
    data_t y_eval,
    data_t x_eval,
    bool neg,
    int sym_mode
) {
#pragma HLS INLINE
    data_t y = y_eval;
    if (neg) {
        if (sym_mode == CAMTA_SYM_ODD) {
            y = static_cast<data_t>(-y_eval);
        }
        else if (sym_mode == CAMTA_SYM_COMPLEMENT) {
            y = static_cast<data_t>(1.0f) - y_eval;
        }
        else if (sym_mode == CAMTA_SYM_RESIDUAL) {
            y = y_eval - x_eval;
        }
    }
    return y;
}
data_t camta_unit(
    data_t x,
    data_t L_left,
    data_t L_right,
    int deg_r0,  int deg_r1,  int deg_r2,
    int mode_r0, int mode_r1, int mode_r2,
    int sym_mode,
    data_t a3_r0, data_t a2_r0, data_t a1_r0, data_t a0_r0,
    data_t a3_r1, data_t a2_r1, data_t a1_r1, data_t a0_r1,
    data_t a3_r2, data_t a2_r2, data_t a1_r2, data_t a0_r2
) {
#pragma HLS PIPELINE II=1
    bool neg = (x < static_cast<data_t>(0));
    data_t x_eval = x;
    if (uses_abs_input(sym_mode)) {
        x_eval = neg ? static_cast<data_t>(-x) : x;
    }
    data_t a3, a2, a1, a0;
    int deg, mode;
    if (x_eval < L_left) {
        a3 = a3_r0;
        a2 = a2_r0;
        a1 = a1_r0;
        a0 = a0_r0;
        deg  = deg_r0;
        mode = mode_r0;
    }
    else if (x_eval > L_right) {
        a3 = a3_r2;
        a2 = a2_r2;
        a1 = a1_r2;
        a0 = a0_r2;
        deg  = deg_r2;
        mode = mode_r2;
    }
    else {
        a3 = a3_r1;
        a2 = a2_r1;
        a1 = a1_r1;
        a0 = a0_r1;
        deg  = deg_r1;
        mode = mode_r1;
    }
    data_t y_eval = eval_region(
        x_eval,
        mode,
        deg,
        a3,
        a2,
        a1,
        a0
    );
    data_t y = apply_symmetry_post(
        y_eval,
        x_eval,
        neg,
        sym_mode
    );
    return y;
}
void camta(
    const data_t x_in[],
    data_t y_out[],
    int n,
    data_t L_left,
    data_t L_right,
    int deg_r0,  int deg_r1,  int deg_r2,
    int mode_r0, int mode_r1, int mode_r2,
    int sym_mode,
    data_t a3_r0, data_t a2_r0, data_t a1_r0, data_t a0_r0,
    data_t a3_r1, data_t a2_r1, data_t a1_r1, data_t a0_r1,
    data_t a3_r2, data_t a2_r2, data_t a1_r2, data_t a0_r2
) {
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
#pragma HLS INTERFACE s_axilite port=sym_mode
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
#pragma HLS STABLE variable=sym_mode
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
    for (int i = 0; i < n; ++i) {
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
            sym_mode,
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
