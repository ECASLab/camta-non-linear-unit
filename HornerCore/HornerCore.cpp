#include <ap_fixed.h>
typedef ap_fixed<16,6> data_t;
typedef ap_fixed<32,12> acc_t;
data_t horner_core(
    data_t x,
    data_t a3,
    data_t a2,
    data_t a1,
    data_t a0)
{
#pragma HLS PIPELINE II=1
#pragma HLS ALLOCATION operation instances=mul limit=1
    acc_t acc = a3;
    for(int i=0;i<3;i++){
#pragma HLS UNROLL off
#pragma HLS PIPELINE
        if(i==0) acc = acc * x + a2;
        if(i==1) acc = acc * x + a1;
        if(i==2) acc = acc * x + a0;
    }
    return (data_t)acc;
}