#include <iostream>
#include <cmath>

#include "../src/camta_types.h"
#include "../src/camta.h"

double reference_function(double x)
{
    return 0.125*x*x*x + 0.25*x*x + 0.5*x + 1.0;
}
 
int main()
{
 
    const int N = 4096;
    double mse = 0.0;

    data_t x_in[N];
    data_t y_out[N];
 
    data_t L = 1.5;
 
    data_t a3 = 0.125;
    data_t a2 = 0.25;
    data_t a1 = 0.5;
    data_t a0 = 1.0;

    int deg_r0 = 3;
    int deg_r1 = 3;
    int deg_r2 = 3;
 
    for(int i=0;i<N;i++)
    {
        double x_d = -4.0 + 8.0 * i / N;
        x_in[i] = x_d;
    }

        camta(x_in, y_out, N, L, deg_r0, deg_r1, deg_r2,
            a3, a2, a1, a0,
            a3, a2, a1, a0,
            a3, a2, a1, a0);

    for(int i=0;i<N;i++)
    {
        double x_d = static_cast<double>(x_in[i]);
        double y_ref = reference_function(x_d);
        double err = static_cast<double>(y_out[i]) - y_ref;
        mse += err*err;
    }
 
    mse /= N;
 
    std::cout<<"MSE = "<<mse<<std::endl;
 
    return 0;
}
