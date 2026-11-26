#ifndef CAMTA_TYPES_H
#define CAMTA_TYPES_H
#include "ap_fixed.h"
// =========================================================================
// PARAMETRIZACIÓN POR DEFECTO: Q5.11 (16 bits totales, 5 bits enteros)
// =========================================================================
constexpr int DEFAULT_DATA_W = 16;  // Total de bits
constexpr int DEFAULT_DATA_I = 5;   // Bits enteros (Q5.11 -> 16 - 5 = 11 frac bits)
constexpr int DEFAULT_ACC_W  = 24;  // Total de bits del acumulador interno
constexpr int DEFAULT_ACC_I  = 9;   // Bits enteros del acumulador (mantiene suficiente rango en multiplicaciones)
// Tipos por defecto usando la configuración Q5.11
typedef ap_fixed<DEFAULT_DATA_W, DEFAULT_DATA_I> data_t;
typedef ap_fixed<DEFAULT_ACC_W, DEFAULT_ACC_I>   acc_t;
// Modos de Operación
static const int CAMTA_MODE_HORNER   = 0;
static const int CAMTA_MODE_CONST    = 1;
static const int CAMTA_MODE_ZERO     = 2;
static const int CAMTA_MODE_IDENTITY = 3;
// Modos de Simetría
static const int CAMTA_SYM_NONE       = 0;
static const int CAMTA_SYM_ODD        = 1;
static const int CAMTA_SYM_COMPLEMENT = 2;
static const int CAMTA_SYM_RESIDUAL   = 3;
static const int CAMTA_SYM_EVEN       = 4;
#endif
