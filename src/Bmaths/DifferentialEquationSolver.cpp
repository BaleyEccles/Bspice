#include "DifferentialEquationSolver.h"

matrix<double> EulersMethod(matrix<double> E, matrix<double> A,
                            matrix<double> f, matrix<double> yn, double h) {
  matrix<double> Einv = E.pseudoInvert();
  std::vector<std::vector<double>> d_yn_dtData;
  std::vector<std::vector<double>> yn1Data;
  auto EinvMulA = multiply(Einv, A);
  for (int row = 0; row < f.rows; row++) {
    auto equEinv = Einv.getRow(row);
    auto equA = A.getRow(row);
    auto equEinvMulA = EinvMulA.getRow(row);
    double equSol1 = 0.0;
    for (int i = 0; i < f.data.size(); i++) {
      equSol1 += equEinv.data[i][0] * f.data[i][0];
    }
    double equSol2 = 0.0;
    for (int i = 0; i < f.data.size(); i++) {
      equSol2 += equEinvMulA.data[i][0] * yn.data[i][0];
    }
    double sol = equSol1 - equSol2;
    double yn1 = h * sol + yn.data[row][0];

    yn1Data.push_back({yn1});
  }

  matrix<double> output = {yn1Data, 1, (int)yn1Data.size()};
  return output;
};
