#pragma once
#include "matrix.h"
#include "AlgebraicEquationSolver.h"
#include "DifferentialEquationSolver.h"

std::pair<std::vector<double>, std::vector<matrix<double>>> DAESolve(matrix<double> A, matrix<double> E, matrix<double> f, matrix<double> initalGuess, double timeStep, double timeEnd);

matrix<double> DAEStepper(matrix<double> A, matrix<double> E, matrix<double> f, matrix<double> yn, double timeStep);


