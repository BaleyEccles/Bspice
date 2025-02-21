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

matrix<double> NewtonsMethod(matrix<multiVaribleFunction> A, matrix<symbol> syms, matrix<double> f, matrix<double> guess);

std::vector<std::pair<symbol, double>> getInputs(matrix<symbol> syms, matrix<double> inputs);
matrix<double> getJacobian(matrix<multiVaribleFunction> funcMatrix, std::vector<std::pair<symbol, double>> inputs, double h);
