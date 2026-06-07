#ifndef CAMTA_H
#define CAMTA_H

#include "camta_types.h"

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
);

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
);

#endif