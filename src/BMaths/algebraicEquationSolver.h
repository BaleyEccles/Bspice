#pragma once
#include "matrix.h"
#include "function.h"

// of the form Ax = f
template<typename T1, typename T2>
struct AlgebraicEquation {
  matrix<T1> A;
  matrix<T2> f;
  matrix<symbol> syms;
};

matrix<double> NewtonsMethod(matrix<double> A, matrix<double> f, matrix<double> guess);

values NewtonsMethod(matrix<multiVaribleFunction> A, matrix<double> f, matrix<symbol> syms, values guess);

matrix<double> getJacobian(matrix<multiVaribleFunction> funcMatrix, values inputs, matrix<symbol> syms, double h);
