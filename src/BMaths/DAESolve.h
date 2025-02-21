#pragma once
#include <array>
#include "matrix.h"
#include "algebraicEquationSolver.h"
#include "differentialEquationSolver.h"
#include "function.h"

template<typename T1, typename T2, typename T3, std::size_t colsT, std::size_t rowsT>
struct DifferentialAlgebraicEquation {
  matrix<T1, colsT, rowsT> A;
  matrix<T2, colsT, rowsT> E;
  matrix<T3, 1, rowsT> f;
  matrix<symbol, 1, rowsT> syms;
};


template<std::size_t colsT, std::size_t rowsT>
std::pair<std::array<double, rowsT>, std::vector<matrix<double, 1, rowsT>>> DAESolve(matrix<double, colsT, rowsT> A, matrix<double, colsT, rowsT> E, matrix<double, 1, rowsT> f, matrix<double, 1, rowsT> initalGuess, double timeStep, double timeEnd);

template<std::size_t colsT, std::size_t rowsT>
matrix<double, colsT, rowsT> getRowsFromIdx(matrix<double, colsT, rowsT> input, std::vector<int>& idx);

template<std::size_t colsT, std::size_t rowsT>
matrix<double, colsT, rowsT> eliminateColsFromIdx(matrix<double, colsT, rowsT> input, std::vector<int>& idx);

template<std::size_t colsT, std::size_t rowsT>
std::vector<int> getDEColIdx(matrix<double, colsT, rowsT> E);

template<std::size_t colsT, std::size_t rowsT>
matrix<double, colsT, rowsT> DAEStepper(matrix<double, colsT, rowsT> A, matrix<double, colsT, rowsT> E, matrix<double, colsT, rowsT> f, matrix<double, colsT, rowsT> yn, double timeStep);



template<typename T1, typename T2, typename T3, std::size_t colsT, std::size_t rowsT>
std::pair<std::vector<double>, std::vector<matrix<double, colsT, rowsT>>> DAESolve2(DifferentialAlgebraicEquation<T1, T2, T3, colsT, rowsT> DAE, matrix<double, colsT, rowsT> initalGuess, double timeStep, double timeEnd) {
  std::vector<matrix<double, colsT, rowsT>> results;
  std::vector<double> time;
  int steps = ceil(timeEnd/timeStep);
  results.reserve(steps);
  auto DEIdx = getDifferentailEquationIdxFromDAE(DAE);
  auto DEColIdx = getDEColIdx(DAE.E);
  auto DEs = getDifferentailEquationsFromDAE(DAE);
  //DEs.E.print("E");
  auto AEIdx = getAlgebraicEquationIdxFromDAE(DAE);
  auto AEs = getAlgebraicEquationsFromDAE(DAE);
  //AEs.A.print("A");
    
  for (int i = 0; i < steps; i++) {
    double tn = i * timeStep - timeStep;
    matrix<double, colsT, rowsT> yn;
    if (results.size() == 0) {
      yn = initalGuess;
    } else {
      yn = results[results.size() - 1];
    }
    
    auto ynDE = getRowsFromIdx(yn, DEIdx);
    matrix<double, colsT, rowsT> xn1;
    if constexpr (std::is_arithmetic<T3>::value) {
      xn1 = add(multiply(DEs.E.invert(), subtract(DEs.f, multiply(DEs.A, yn))).scale(timeStep), ynDE);

    } else if constexpr (std::is_same<T3, function>::value) {
      matrix<double, colsT, rowsT> DEsfEval = DEs.f.evaluate(tn);
      //auto E2 = DEs.E.scale(0.25);
      xn1 = ((DEs.E.invert() * (DEsfEval - (DEs.A * yn))).scale(timeStep) + ynDE);
    }

    auto An = eliminateColsFromIdx(AEs.A, DEColIdx);

    matrix<double, 1, rowsT> xn1New;
    int j = 0;
    for (auto row : DEIdx) {
      xn1New.data[row][0] = xn1.data[j][0];
      j++;
    }
    auto An1xn1 = (AEs.A * xn1New);
    matrix<double, colsT, rowsT> newf;
    if constexpr (std::is_arithmetic<T3>::value) {
      newf = subtract(AEs.f, An1xn1);
    } else if constexpr (std::is_same<T3, function>::value) {
      auto AEfEval = AEs.f.evaluate(tn);
      newf = (AEfEval - An1xn1);
    }

    auto NewtonGuess = yn;
    j = 0;
    for (auto row : DEIdx) {
      NewtonGuess.eliminateRow(row - j);
      j++;
    }
    //newf.print("newf");
    auto AEsols = NewtonsMethod(An, newf, NewtonGuess);

    matrix<double, colsT, rowsT> AEsolsNew;

    j = 0;
    for (auto row : AEIdx) {
      AEsolsNew.data[row][0] = AEsols.data[j][0];
      j++;
    }
    auto nextStep = AEsolsNew + xn1New;
    results.push_back(nextStep);
    time.push_back(tn);
  };
  
  std::vector<matrix<double, colsT, rowsT>> resultsReformated;
  for (auto& r : results) {
    matrix<double, colsT, rowsT> m;
    resultsReformated.push_back(m);
  }
  for (auto& r : results) {
    for (int row = 0; row < results[0].rows; row++) {
      //TODO:      resultsReformated[row].data[0].push_back(r.data[row][0]);
      resultsReformated[row].cols++;
    }
  }

  auto output = std::pair<std::vector<double>, std::vector<matrix<double, colsT, rowsT>>>{time, resultsReformated};
  return output;
}


template<typename T1, typename T2, typename T3, std::size_t colsT, std::size_t rowsT>
std::vector<int> getDifferentailEquationIdxFromDAE(DifferentialAlgebraicEquation<T1, T2, T3, colsT, rowsT> DAE) {
  std::vector<int> DERowIdx;
  for (int row = 0; row < DAE.E.rows; ++row) {
    bool isDE = false;
    for (int col = 0; col < DAE.E.cols; ++col) {
      if (DAE.E.data[row][col] != 0.0) {
        isDE = true;
      }
    }
    if (isDE) {
      DERowIdx.push_back(row);
    }
  }
  return DERowIdx;
}


template<typename T1, typename T2, typename T3, std::size_t colsT, std::size_t rowsT>
std::vector<int> getAlgebraicEquationIdxFromDAE(DifferentialAlgebraicEquation<T1, T2, T3, colsT, rowsT> DAE) {
  std::vector<int> AERowIdx;
  for (int row = 0; row < DAE.E.rows; ++row) {
    bool isDE = true;
    for (int col = 0; col < DAE.E.cols; ++col) {
      if (DAE.E.data[row][col] != 0.0) {
        isDE = false;
      }
    }
    if (isDE) {
      AERowIdx.push_back(row);
    }
  }
  return AERowIdx;
}




template<typename T1, typename T2, typename T3, std::size_t colsT, std::size_t rowsT>
DifferentialEquation<T1, T2, T3, colsT, rowsT> getDifferentailEquationsFromDAE(DifferentialAlgebraicEquation<T1, T2, T3, colsT, rowsT> DAE) {
  auto DERowIdx = getDifferentailEquationIdxFromDAE(DAE);
  //auto AERowIdx = getAlgebraicEquationIdxFromDAE(DAE);
  matrix<T1, colsT, rowsT> ADE; //  = {std::vector<std::vector<T1>>{}, DAE.A.cols, (int)DERowIdx.size()};
  matrix<T2, colsT, rowsT> EDE; //  = {std::vector<std::vector<T2>>{}, DAE.E.cols, (int)DERowIdx.size()};
  matrix<T3, 1, rowsT> fDE; // = {std::vector<std::vector<T3>>{}, DAE.f.cols, (int)DERowIdx.size()};
  matrix<symbol, 1, rowsT> symsDE; // = {std::vector<std::vector<symbol>>{}, DAE.syms.cols, (int)DERowIdx.size()};
  int i = 0;
  for (auto& row : DERowIdx) {
    EDE.data[i] = (DAE.E.getRow(row).transpose().data[0]);
    ADE.data[i] = (DAE.A.getRow(row).transpose().data[0]);
    fDE.data[i] = (DAE.f.getRow(row).transpose().data[0]);
    symsDE.data[i] = (DAE.syms.getRow(row).transpose().data[0]);
    i++;
  }


  std::vector<int> AEColIdx;
  for (int col = 0; col < DAE.E.cols; col++) {
    bool isZero = true;
    for (int row = 0; row < DAE.E.rows; row++) {
      if (DAE.E.data[row][col] != 0.0) {
        isZero = false;
      }
    }
    if (isZero) {
      AEColIdx.push_back(col);
    }
  }
  i = 0;
  for (auto col : AEColIdx) {
    EDE.eliminateCol(col - i);
    i++;
  }
  
  DifferentialEquation<T1, T2, T3, colsT, rowsT> DE;
  DE.E = EDE;
  DE.A = ADE;
  DE.f = fDE;
  DE.syms = symsDE;
  return DE;
}


template<typename T1, typename T2, typename T3, std::size_t colsT, std::size_t rowsT>
AlgebraicEquation<T1, T2, colsT, rowsT> getAlgebraicEquationsFromDAE(DifferentialAlgebraicEquation<T1, T2, T3, colsT, rowsT> DAE) {
  auto AERowIdx = getAlgebraicEquationIdxFromDAE(DAE);
  matrix<T3, colsT, rowsT> EAE; // = {std::vector<std::vector<T3>>{}, DAE.E.cols, (int)AERowIdx.size()};
  matrix<T1, colsT, rowsT> AAE; // = {std::vector<std::vector<T1>>{}, DAE.A.cols, (int)AERowIdx.size()};
  matrix<T2, 1, rowsT> fAE; // = {std::vector<std::vector<T2>>{}, DAE.f.cols, (int)AERowIdx.size()};
  matrix<symbol, 1, rowsT> symsAE; // = {std::vector<std::vector<symbol>>{}, DAE.syms.cols, (int)AERowIdx.size()};
  int i = 0;
  for (auto& row : AERowIdx) {
    //EAE.data[i] = (DAE.E.getRow(row).transpose().data[0]);
    //AAE.data[i] = (DAE.A.getRow(row).transpose().data[0]);
    //fAE.data[i] = (DAE.f.getRow(row).transpose().data[0]);
    //symsAE.data[i] = DAE.syms.getRow(row).transpose().data[0]; TODO
    i++;
  }
  AlgebraicEquation<T1, T2, colsT, rowsT> AE;
  AE.A = AAE;
  AE.f = fAE;
  AE.syms = symsAE;
  return AE;
}

