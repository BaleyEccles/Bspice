#pragma once
#include "matrix.h"

// of the form Ax = f
template<typename T1, typename T2>
struct AlgebraicEquation {
  matrix<T1> A;
  matrix<T2> f;
  matrix<symbol> syms;
};

matrix<double> NewtonsMethod(matrix<double> A, matrix<double> f, matrix<double> guess);
