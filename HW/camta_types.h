#ifndef CAMTA_TYPES_H
#define CAMTA_TYPES_H

#include "ap_fixed.h"

typedef ap_fixed<16,6> data_t;
typedef ap_fixed<24,10> acc_t;

static const int CAMTA_MODE_HORNER   = 0;
static const int CAMTA_MODE_CONST    = 1;
static const int CAMTA_MODE_ZERO     = 2;
static const int CAMTA_MODE_IDENTITY = 3;

#endif