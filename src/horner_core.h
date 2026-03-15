#ifndef HORNER_CORE_H
#define HORNER_CORE_H
#include "ap_fixed.h"
typedef ap_fixed<16,6> data_t;
typedef ap_fixed<24,10> acc_t;
data_t horner_core(
    data_t x,
    data_t a3,
    data_t a2,
    data_t a1,
    data_t a0);
#endif