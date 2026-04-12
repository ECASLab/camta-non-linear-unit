#ifndef CAMTA_TYPES_H
#define CAMTA_TYPES_H

#include "ap_fixed.h"
#include "ap_int.h"

typedef ap_fixed<16,6> data_t;
typedef ap_fixed<24,10> acc_t;
typedef ap_uint<512> word_t;

static const int CAMTA_AXI_BITS  = 512;
static const int CAMTA_DATA_BITS = 16;
static const int CAMTA_LANES     = CAMTA_AXI_BITS / CAMTA_DATA_BITS;

static inline data_t camta_unpack_lane(const word_t& word, int lane)
{
#pragma HLS INLINE
    ap_uint<CAMTA_DATA_BITS> bits = word.range((lane + 1) * CAMTA_DATA_BITS - 1,
                                               lane * CAMTA_DATA_BITS);
    data_t value;
    value.range(CAMTA_DATA_BITS - 1, 0) = bits;
    return value;
}

static inline void camta_pack_lane(word_t& word, int lane, data_t value)
{
#pragma HLS INLINE
    ap_uint<CAMTA_DATA_BITS> bits = value.range(CAMTA_DATA_BITS - 1, 0);
    word.range((lane + 1) * CAMTA_DATA_BITS - 1,
               lane * CAMTA_DATA_BITS) = bits;
}

#endif
