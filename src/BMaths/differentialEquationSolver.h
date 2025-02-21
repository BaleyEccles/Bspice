#include "matrix.h"

// of the form Ax + Ex' = f
template<typename T1, typename T2, typename T3, std::size_t colsT, std::size_t rowsT>
struct DifferentialEquation {
  matrix<T1, colsT, rowsT> A;
  matrix<T2, colsT, rowsT> E;
  matrix<T3, 1, rowsT> f;
  matrix<symbol, 1, rowsT> syms;
};
template<std::size_t colsT, std::size_t rowsT>
matrix<double, colsT, rowsT> EulersMethod(matrix<double, colsT, rowsT> E, matrix<double, colsT, rowsT> A, matrix<double, 1, rowsT> f, matrix<double, 1, rowsT> yn, double h);
