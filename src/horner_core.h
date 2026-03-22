#ifndef HORNER_CORE_H
#define HORNER_CORE_H

#include "camta_types.h"

data_t horner_core(
    data_t x,
    int degree,
    data_t a3,
    data_t a2,
    data_t a1,
    data_t a0);
#endif