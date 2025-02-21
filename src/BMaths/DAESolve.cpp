#include "DAESolve.h"
#include "function.h"

// Solve a DAE of the form F = f1
// where F is a vector of multivarible functions that do not contain derivatives
// and f is a vector of constants, or a vector of single varible functions
// AND E = f2
// where E is a vector of multivarible functions that contains derivatives
// and f is a vector of constants, or a vector of single varible functions
std::pair<std::vector<double>, std::vector<std::pair<symbol, std::vector<double>>>>
DAESolve3(matrix<multiVaribleFunction> F, matrix<multiVaribleFunction> E, matrix<function> f1, matrix<function> f2, matrix<symbol> syms, std::vector<std::pair<symbol, double>> initalGuess, double timeStep, double timeEnd) {
  std::vector<std::vector<std::pair<symbol, double>>> results;
  std::vector<double> time;
  int steps = ceil(timeEnd/timeStep);


  for (int i = 0; i < steps; i++) {
    double time = i * timeStep - timeStep;
    std::vector<std::pair<symbol, double>> currentStep;
    if (results.size() == 0) {
      currentStep = initalGuess;
    } else {
      currentStep = results[results.size() - 1];
    }
    auto EEval = E.evaluate(currentStep);
    auto f2Eval = f2.evaluate(time);
    // DE stuff

    auto xn1 = NewtonsMethod(E, syms, f2.evaluate(time), currentStep); // FIXME: f2.evaluate is using the other eval
    
  }
}


// Solve a DAE of the form Ax + Ex' = f
std::pair<std::vector<double>, std::vector<matrix<double>>> DAESolve(matrix<double> A, matrix<double> E, matrix<double> f, matrix<double> initalGuess, double timeStep, double timeEnd) {
  std::vector<matrix<double>> results;
  std::vector<double> time;
  int steps = ceil(timeEnd/timeStep);
  results.reserve(steps);
  for (int i = 0; i < steps; i++) {
    double tn = i * timeStep - timeStep;
    matrix<double> yn;
    if (results.size() == 0) {
      yn = initalGuess;
    } else {
      yn = results[results.size() - 1];
    }

    auto nextStep = DAEStepper(A, E, f, yn, timeStep);
    results.push_back(nextStep);
    time.push_back(tn);
  };
  auto output = std::pair<std::vector<double>, std::vector<matrix<double>>>{time, results};
  return output;
}

matrix<double> getRowsFromIdx(matrix<double> input, std::vector<int>& idx) {
  matrix<double> output = {std::vector<std::vector<double>>{}, input.cols, (int)idx.size()};
  for (auto& row : idx) {
    output.data.push_back(input.getRow(row).transpose().data[0]);
  }
  return output;
}

std::vector<int> getDEColIdx(matrix<double> E) {
  std::vector<int> DEColIdx;
  for (int col = 0; col < E.cols; col++) {
    bool isZero = false;
    for (int row = 0; row < E.rows; row++) {
      if (E.data[row][col] != 0.0) {
        isZero = true;
      }
    }
    if (isZero) {
      DEColIdx.push_back(col);
    }
  }
  return DEColIdx;
}

matrix<double> eliminateColsFromIdx(matrix<double> input, std::vector<int>& idx) {
  int i = 0;
  for (auto col : idx) {
    input.eliminateCol(col - i);
    i++;
  }
  return input;
}

matrix<double> DAEStepper(matrix<double> A, matrix<double> E, matrix<double> f,
                          matrix<double> yn, double timeStep) {
  std::vector<int> DERowIdx;

  for (int row = 0; row < E.rows; ++row) {
    bool isDE = false;
    for (int col = 0; col < E.cols; ++col) {
      if (E.data[row][col] != 0.0) {
        isDE = true;
      }
    }
    if (isDE) {
      DERowIdx.push_back(row);
    }
  }
  
  std::vector<int> AERowIdx;

  for (int row = 0; row < E.rows; ++row) {
    bool isAE = true;
    for (int col = 0; col < E.cols; ++col) {
      if (E.data[row][col] != 0.0) {
        isAE = false;
      }
    }
    if (isAE) {
      AERowIdx.push_back(row);
    }
  }
  
  matrix<double> EDE = {std::vector<std::vector<double>>{}, E.cols, (int)DERowIdx.size()};
  matrix<double> ADE = {std::vector<std::vector<double>>{}, A.cols, (int)DERowIdx.size()};
  matrix<double> fDE = {std::vector<std::vector<double>>{}, f.cols, (int)DERowIdx.size()};
  matrix<double> ynDE = {std::vector<std::vector<double>>{}, yn.cols, (int)DERowIdx.size()};

  int i = 0;
  for (auto row : DERowIdx) {
    EDE.data.push_back(E.getRow(row).transpose().data[0]);
    ADE.data.push_back(A.getRow(row).transpose().data[0]);
    fDE.data.push_back(f.getRow(row).transpose().data[0]);
    ynDE.data.push_back(yn.getRow(row).transpose().data[0]);
  }

  std::vector<int> AEColIdx;
  for (int col = 0; col < E.cols; col++) {
    bool isZero = true;
    for (int row = 0; row < E.rows; row++) {
      if (E.data[row][col] != 0.0) {
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
  //EDE.print();

  auto xn1 = ((EDE.invert() * (fDE -(ADE * yn))).scale(timeStep) + ynDE);
  //xn1.print();

  matrix<double> xn1New = {std::vector<std::vector<double>>(f.rows, std::vector<double>(f.cols, 0.0)), f.cols, f.rows};
  i = 0;
  for (auto row : DERowIdx) {
    xn1New.data[row][0] = xn1.data[i][0];
    i++;
  }
  
  // AE Equations
  matrix<double> An1 = A;
  i = 0;
  for (auto row : DERowIdx) {
    An1.eliminateRow(row - i);
    i++;
  }

  std::vector<int> DEColIdx;
  for (int col = 0; col < E.cols; col++) {
    bool isZero = false;
    for (int row = 0; row < E.rows; row++) {
      if (E.data[row][col] != 0.0) {
        isZero = true;
      }
    }
    if (isZero) {
      DEColIdx.push_back(col);
    }
  }
  
  auto An = An1;
  i = 0;
  for (auto col : DEColIdx) {
    An.eliminateCol(col - i);
    i++;
  }
  // Now we need to solve the equation:
  // An xn = f - An1 xn1
  auto fAE = f;
  i = 0;
  for (auto row : DERowIdx) {
    fAE.eliminateRow(row - i);
    i++;
  }
  auto An1xn1 = (An1 * xn1New);
  auto newf = (fAE - An1xn1);
  auto NewtonGuess = yn;
  i = 0;
  for (auto row : DERowIdx) {
    NewtonGuess.eliminateRow(row - i);
    i++;
  }

  auto AEsols = NewtonsMethod(An, newf, NewtonGuess);

  matrix<double> AEsolsNew = {std::vector<std::vector<double>>(
                                                               f.rows, std::vector<double>(f.cols, 0.0)),
                              f.cols, f.rows};

  i = 0;
  for (auto row : AERowIdx) {
    AEsolsNew.data[row][0] = AEsols.data[i][0];
    i++;
  }
  auto output = (AEsolsNew + xn1New);

  return output;
  
}
