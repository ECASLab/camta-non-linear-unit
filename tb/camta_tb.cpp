#include <iostream>
#include <cmath>
#include "../src/camta.h"
 
double reference_function(double x)
{
    return 0.125*x*x*x + 0.25*x*x + 0.5*x + 1.0;
}
 
int main()
{
 
    const int N = 1000;
    double mse = 0.0;
 
    data_t L = 1.5;
 
    data_t a3 = 0.125;
    data_t a2 = 0.25;
    data_t a1 = 0.5;
    data_t a0 = 1.0;
 
    for(int i=0;i<N;i++)
    {
 
        double x_d = -4.0 + 8.0 * i / N;
        data_t x = x_d;
 
        data_t y_hw = camta(
            x,L,
            a3,a2,a1,a0,
            a3,a2,a1,a0,
            a3,a2,a1,a0
        );
 
        double y_ref = reference_function(x_d);
 
        double err = (double)y_hw - y_ref;
 
        mse += err*err;
    }
 
    mse /= N;
 
    std::cout<<"MSE = "<<mse<<std::endl;
 
    return 0;
}