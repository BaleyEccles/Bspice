#include "matrix.h"

// of the form Ax + Ex' = f
template<typename T1, typename T2, typename T3>
struct DifferentialEquation {
  matrix<T1> A;
  matrix<T2> E;
  matrix<T3> f;
  matrix<symbol> syms;
};

matrix<double> EulersMethod(matrix<double> E, matrix<double> A, matrix<double> f, matrix<double> yn, double h);
