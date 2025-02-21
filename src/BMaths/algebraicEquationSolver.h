#pragma once
#include "matrix.h"

// of the form Ax = f
template<typename T1, typename T2, std::size_t colsT, std::size_t rowsT>
struct AlgebraicEquation {
  matrix<T1, colsT, rowsT> A;
  matrix<T2, colsT, rowsT> f;
  matrix<symbol, 1, rowsT> syms;
};

template<std::size_t colsT, std::size_t rowsT>
matrix<double, colsT, rowsT> NewtonsMethod(matrix<double, colsT, rowsT> A, matrix<double, 1, rowsT> f, matrix<double, 1, rowsT> guess);

