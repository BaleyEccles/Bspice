#pragma once
#include "matrix.h"
#include "algebraicEquationSolver.h"
#include "differentialEquationSolver.h"
#include "function.h"

template<typename T1, typename T2, typename T3>
struct DifferentialAlgebraicEquation {
  matrix<T1> A;
  matrix<T2> E;
  matrix<T3> f;
  matrix<symbol> syms;
};



std::pair<std::vector<double>, std::vector<matrix<double>>> DAESolve(matrix<double> A, matrix<double> E, matrix<double> f, matrix<double> initalGuess, double timeStep, double timeEnd);


matrix<double> getRowsFromIdx(matrix<double> input, std::vector<int>& idx);
matrix<double> eliminateColsFromIdx(matrix<double> input, std::vector<int>& idx);
std::vector<int> getDEColIdx(matrix<double> E);

matrix<double> DAEStepper(matrix<double> A, matrix<double> E, matrix<double> f, matrix<double> yn, double timeStep);


// Solve a DAE of the form F + E' = f
// where F is a vector of multivarible functions that do not contain derivatives
// E' is a vector of multivarible functions that are derivatives of the varibles in F
// and f is a vector of constants, or a vector of single varible functions
// There is a limitation on the relation from E' and F, that is they must be added.
// EX: The equation x*dx/dt could not be solved. I dont belive there are any circuits that encounter this limitation.
template<typename T>
std::pair<std::vector<double>, std::vector<values>> DAESolveNonLinear(DifferentialAlgebraicEquation<multiVaribleFunction, multiVaribleFunction, T> DAE, values initalGuess, double timeStep, double timeEnd) {
  std::vector<values> results;
  std::vector<double> time;
  int steps = ceil(timeEnd/timeStep);
  double& h = timeStep;
  auto& F = DAE.A;
  auto& E = DAE.E;
  auto& f = DAE.f;
  auto& syms = DAE.syms;

  for (int i = 0; i < steps; i++) {
    double time = i * timeStep - timeStep;
    matrix<double> currentStepMatrix;
    if (results.size() == 0) {
      currentStepMatrix = valuesToMatrix<double>(initalGuess, syms);
    } else {
      currentStepMatrix = valuesToMatrix<double>(results[results.size() - 1], syms);
    }
    auto currentStepValues = matrixToValues(currentStepMatrix, syms);

    // DE solving
    auto FEval = F.evaluate(currentStepValues);
    auto fEval = f.evaluate(time);
    // E' = f - F
    auto fSubF = fEval - FEval;
    //    (E'_{n+1} - E'_{n})/h = f - F
    // => E'_{n+1}= h*(f - F) + E'_{n}
    auto En1 = fSubF.scale(h) + E.evaluate(currentStepValues);

    // Algebraic equations solving
    auto Fn1 = NewtonsMethod(F, f.evaluate(time), syms, currentStepValues);
    
  }
}



template<typename T>
std::pair<std::vector<double>, std::vector<matrix<double>>> DAESolveLinear(DifferentialAlgebraicEquation<double, double, T> DAE, values initalGuess, double timeStep, double timeEnd) {
  std::vector<matrix<double>> results;
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
  auto& syms = DAE.syms;
    
  for (int i = 0; i < steps; i++) {
    double tn = i * timeStep - timeStep;
    matrix<double> currentStep;
    if (results.size() == 0) {
      currentStep = valuesToMatrix<double>(initalGuess, syms);
    } else {
      currentStep = results[results.size() - 1];
    }
    
    auto currentStepDE = getRowsFromIdx(currentStep, DEIdx);
    matrix<double> xn1;
    if constexpr (std::is_arithmetic<T>::value) {
      xn1 = add(multiply(DEs.E.invert(), subtract(DEs.f, multiply(DEs.A, currentStep))).scale(timeStep), currentStepDE);

    } else if constexpr (std::is_same<T, function>::value) {
      matrix<double> DEsfEval = DEs.f.evaluate(tn);
      //auto E2 = DEs.E.scale(0.25);
      xn1 = ((DEs.E.invert() * (DEsfEval - (DEs.A * currentStep))).scale(timeStep) + currentStepDE);
    }

    auto An = eliminateColsFromIdx(AEs.A, DEColIdx);

    matrix<double> xn1New = {std::vector<std::vector<double>>(DAE.f.rows, std::vector<double>(DAE.f.cols, 0.0)), DAE.f.cols, DAE.f.rows};
    int j = 0;
    for (auto row : DEIdx) {
      xn1New.data[row][0] = xn1.data[j][0];
      j++;
    }
    auto An1xn1 = (AEs.A * xn1New);
    matrix<double> newf;
    if constexpr (std::is_arithmetic<T>::value) {
      newf = subtract(AEs.f, An1xn1);
    } else if constexpr (std::is_same<T, function>::value) {
      auto AEfEval = AEs.f.evaluate(tn);
      newf = (AEfEval - An1xn1);
    }

    auto NewtonGuess = currentStep;
    j = 0;
    for (auto row : DEIdx) {
      NewtonGuess.eliminateRow(row - j);
      j++;
    }
    //newf.print("newf");
    auto AEsols = NewtonsMethod(An, newf, NewtonGuess);

    matrix<double> AEsolsNew = {std::vector<std::vector<double>>(
                                                                 DAE.f.rows, std::vector<double>(DAE.f.cols, 0.0)),
                                DAE.f.cols, DAE.f.rows};

    j = 0;
    for (auto row : AEIdx) {
      AEsolsNew.data[row][0] = AEsols.data[j][0];
      j++;
    }
    auto nextStep = AEsolsNew + xn1New;
    results.push_back(nextStep);
    time.push_back(tn);
  };
  
  std::vector<matrix<double>> resultsReformated;
  for (auto& r : results) {
    matrix<double> m = {
      {{}},
      0, 1
    };
    resultsReformated.push_back(m);
  }
  for (auto& r : results) {
    for (int row = 0; row < results[0].rows; row++) {
      resultsReformated[row].data[0].push_back(r.data[row][0]);
      resultsReformated[row].cols++;
    }
  }

  auto output = std::pair<std::vector<double>, std::vector<matrix<double>>>{time, resultsReformated};
  return output;
}


template<typename T1, typename T2, typename T3>
std::vector<int> getDifferentailEquationIdxFromDAE(DifferentialAlgebraicEquation<T1, T2, T3> DAE) {
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


template<typename T1, typename T2, typename T3>
std::vector<int> getAlgebraicEquationIdxFromDAE(DifferentialAlgebraicEquation<T1, T2, T3> DAE) {
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




// TODO: Template this
template<typename T1, typename T2, typename T3>
DifferentialEquation<T1, T2, T3> getDifferentailEquationsFromDAE(DifferentialAlgebraicEquation<T1, T2, T3> DAE) {
  auto DERowIdx = getDifferentailEquationIdxFromDAE(DAE);
  //auto AERowIdx = getAlgebraicEquationIdxFromDAE(DAE);
  matrix<T1> ADE = {std::vector<std::vector<T1>>{}, DAE.A.cols, (int)DERowIdx.size()};
  matrix<T2> EDE = {std::vector<std::vector<T2>>{}, DAE.E.cols, (int)DERowIdx.size()};
  matrix<T3> fDE = {std::vector<std::vector<T3>>{}, DAE.f.cols, (int)DERowIdx.size()};
  matrix<symbol> symsDE = {std::vector<std::vector<symbol>>{}, DAE.syms.cols, (int)DERowIdx.size()};

  for (auto& row : DERowIdx) {
    EDE.data.push_back(DAE.E.getRow(row).transpose().data[0]);
    ADE.data.push_back(DAE.A.getRow(row).transpose().data[0]);
    fDE.data.push_back(DAE.f.getRow(row).transpose().data[0]);
    symsDE.data.push_back(DAE.syms.getRow(row).transpose().data[0]);
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
  int i = 0;
  for (auto col : AEColIdx) {
    EDE.eliminateCol(col - i);
    i++;
  }
  
  DifferentialEquation<T1, T2, T3> DE;
  DE.E = EDE;
  DE.A = ADE;
  DE.f = fDE;
  DE.syms = symsDE;
  return DE;
}


template<typename T1, typename T2, typename T3>
AlgebraicEquation<T1, T2> getAlgebraicEquationsFromDAE(DifferentialAlgebraicEquation<T1, T3, T2> DAE) {
  auto AERowIdx = getAlgebraicEquationIdxFromDAE(DAE);
  matrix<T3> EAE = {std::vector<std::vector<T3>>{}, DAE.E.cols, (int)AERowIdx.size()};
  matrix<T1> AAE = {std::vector<std::vector<T1>>{}, DAE.A.cols, (int)AERowIdx.size()};
  matrix<T2> fAE = {std::vector<std::vector<T2>>{}, DAE.f.cols, (int)AERowIdx.size()};
  matrix<symbol> symsAE = {std::vector<std::vector<symbol>>{}, DAE.syms.cols, (int)AERowIdx.size()};

  for (auto& row : AERowIdx) {
    EAE.data.push_back(DAE.E.getRow(row).transpose().data[0]);
    AAE.data.push_back(DAE.A.getRow(row).transpose().data[0]);
    fAE.data.push_back(DAE.f.getRow(row).transpose().data[0]);
    symsAE.data.push_back(DAE.syms.getRow(row).transpose().data[0]);
  }
  AlgebraicEquation<T1, T2> AE;
  AE.A = AAE;
  AE.f = fAE;
  AE.syms = symsAE;
  return AE;
}

