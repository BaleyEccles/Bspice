#pragma once
#include "matrix.h"
#include "AlgebraicEquationSolver.h"
#include "DifferentialEquationSolver.h"
#include "function.h"

template<typename T1, typename T2, typename T3>
struct DifferentialAlgebraicEquation {
  matrix<T1> A;
  matrix<T2> E;
  matrix<T3> f;
  matrix<symbol> syms;
};



std::pair<std::vector<double>, std::vector<matrix<double>>> DAESolve(matrix<double> A, matrix<double> E, matrix<double> f, matrix<double> initalGuess, double timeStep, double timeEnd);

template<typename T1, typename T2, typename T3>
std::pair<std::vector<double>, std::vector<matrix<double>>> DAESolve2(DifferentialAlgebraicEquation<T1, T2, T3> DAE, matrix<double> initalGuess, double timeStep, double timeEnd);

// Idx of the DEs/AEs and values at those regions
template<typename T1, typename T2, typename T3>
DifferentialEquation<T1, T2, T3> getDifferentailEquationsFromDAE(DifferentialAlgebraicEquation<T1, T2, T3> DAE);
template<typename T1, typename T2, typename T3>
std::vector<int> getDifferentailEquationIdxFromDAE(DifferentialAlgebraicEquation<T1, T2, T3> DAE);
  
template<typename T1, typename T2, typename T3>
AlgebraicEquation<T1, T2> getAlgebraicEquationsFromDAE(DifferentialAlgebraicEquation<T1, T3, T2> DAE);
template<typename T1, typename T2, typename T3>
std::vector<int> getAlgebraicEquationIdxFromDAE(DifferentialAlgebraicEquation<T1, T2, T3> DAE);

matrix<double> getRowsFromIdx(matrix<double> input, std::vector<int>& idx);
matrix<double> eliminateColsFromIdx(matrix<double> input, std::vector<int>& idx);
std::vector<int> getDEColIdx(matrix<double> E);

matrix<double> DAEStepper(matrix<double> A, matrix<double> E, matrix<double> f, matrix<double> yn, double timeStep);


#include "DAESolve_impl.h"
