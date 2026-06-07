#include <iostream>
#include <cmath>

#include "../src/camta_types.h"
#include "../src/camta.h"

double reference_function(double x)
{
    if (x < -1.0)
    {
        return -1.0;
    }
    else if (x > 1.0)
    {
        return 1.0;
    }
    else
    {
        return x;
    }
}
 
int main()
{
    const int N = 4096;
    double mse = 0.0;

    data_t x_in[N];
    data_t y_out[N];
 
    data_t L_left  = -1.0;
    data_t L_right =  1.0;

    int deg_r0 = 0;
    int deg_r1 = 1;
    int deg_r2 = 0;

    int mode_r0 = CAMTA_MODE_CONST;
    int mode_r1 = CAMTA_MODE_HORNER;
    int mode_r2 = CAMTA_MODE_CONST;

    data_t a3_r0 = 0.0;
    data_t a2_r0 = 0.0;
    data_t a1_r0 = 0.0;
    data_t a0_r0 = -1.0;

    data_t a3_r1 = 0.0;
    data_t a2_r1 = 0.0;
    data_t a1_r1 = 1.0;
    data_t a0_r1 = 0.0;

    data_t a3_r2 = 0.0;
    data_t a2_r2 = 0.0;
    data_t a1_r2 = 0.0;
    data_t a0_r2 = 1.0;
 
    for (int i = 0; i < N; i++)
    {
        double x_d = -4.0 + 8.0 * i / N;
        x_in[i] = static_cast<data_t>(x_d);
        y_out[i] = static_cast<data_t>(0);
    }

    camta(
        x_in,
        y_out,
        N,
        L_left,
        L_right,
        deg_r0,
        deg_r1,
        deg_r2,
        mode_r0,
        mode_r1,
        mode_r2,
        a3_r0, a2_r0, a1_r0, a0_r0,
        a3_r1, a2_r1, a1_r1, a0_r1,
        a3_r2, a2_r2, a1_r2, a0_r2
    );

    for (int i = 0; i < N; i++)
    {
        double x_d = static_cast<double>(x_in[i]);
        double y_ref = reference_function(x_d);
        double err = static_cast<double>(y_out[i]) - y_ref;
        mse += err * err;
    }
 
    mse /= N;
 
    std::cout << "MSE = " << mse << std::endl;
 
    return 0;
}