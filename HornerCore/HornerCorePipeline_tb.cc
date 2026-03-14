#include <iostream>
#include <cmath>
#include "ap_fixed.h"

typedef ap_fixed<16,6> fixed_t;

// Declaración del core
fixed_t horner_core_pipeline(
    fixed_t x,
    fixed_t a3,
    fixed_t a2,
    fixed_t a1,
    fixed_t a0);

double reference_function(double x) {
    // Ejemplo: aproximación dummy (puedes cambiarla)
    return 0.125*x*x*x + 0.25*x*x + 0.5*x + 1.0;
}

int main() {

    const int N = 1000;
    double mse = 0.0;

    fixed_t a3 = 0.125;
    fixed_t a2 = 0.25;
    fixed_t a1 = 0.5;
    fixed_t a0 = 1.0;

    for(int i = 0; i < N; i++) {

        double x_d = -4.0 + 8.0 * i / N;
        fixed_t x = x_d;

        fixed_t y_fixed = horner_core_pipeline(x, a3, a2, a1, a0);
        double y_ref = reference_function(x_d);

        double error = (double)y_fixed - y_ref;
        mse += error * error;
    }

    mse /= N;

    std::cout << "MSE = " << mse << std::endl;

    return 0;
}