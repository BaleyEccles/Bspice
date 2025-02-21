#include "DAESolve.h"


// Solve a DAE of the form Ax + Ex' = f
template <std::size_t colsT, std::size_t rowsT>
std::pair<std::vector<double>, std::vector<matrix<double, colsT, rowsT>>> DAESolve(matrix<double, colsT, rowsT> A, matrix<double, colsT, rowsT> E, matrix<double, 1, rowsT> f, matrix<double, 1, rowsT> initalGuess, double timeStep, double timeEnd) {
  std::vector<matrix<double, colsT, rowsT>> results;
  std::vector<double> time;
  int steps = ceil(timeEnd/timeStep);
  results.reserve(steps);
  for (int i = 0; i < steps; i++) {
    double tn = i * timeStep - timeStep;
    matrix<double, colsT, rowsT> yn;
    if (results.size() == 0) {
      yn = initalGuess;
    } else {
      yn = results[results.size() - 1];
    }

    auto nextStep = DAEStepper(A, E, f, yn, timeStep);
    results.push_back(nextStep);
    time.push_back(tn);
  };
  auto output = std::pair<std::vector<double>, std::vector<matrix<double, colsT, rowsT>>>{time, results};
  return output;
}

template <std::size_t colsT, std::size_t rowsT>
matrix<double, colsT, rowsT> getRowsFromIdx(matrix<double, colsT, rowsT> input, std::vector<int>& idx) {
  matrix<double, colsT, rowsT> output = {std::vector<std::vector<double>>{}, input.cols, (int)idx.size()};
  for (auto& row : idx) {
    output.data.push_back(input.getRow(row).transpose().data[0]);
  }
  return output;
}

template <std::size_t colsT, std::size_t rowsT>
std::vector<int> getDEColIdx(matrix<double, colsT, rowsT> E) {
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

template <std::size_t colsT, std::size_t rowsT>
matrix<double, colsT, rowsT> eliminateColsFromIdx(matrix<double, colsT, rowsT> input, std::vector<int>& idx) {
  int i = 0;
  for (auto col : idx) {
    input.eliminateCol(col - i);
    i++;
  }
  return input;
}
template <std::size_t colsT, std::size_t rowsT>
matrix<double, colsT, rowsT> DAEStepper(matrix<double, colsT, rowsT> A, matrix<double, colsT, rowsT> E, matrix<double, colsT, rowsT> f,
                                  matrix<double, colsT, rowsT> yn, double timeStep) {
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
  
  matrix<double, colsT, rowsT> EDE = {std::vector<std::vector<double>>{}, E.cols, (int)DERowIdx.size()};
  matrix<double, colsT, rowsT> ADE = {std::vector<std::vector<double>>{}, A.cols, (int)DERowIdx.size()};
  matrix<double, colsT, rowsT> fDE = {std::vector<std::vector<double>>{}, f.cols, (int)DERowIdx.size()};
  matrix<double, colsT, rowsT> ynDE = {std::vector<std::vector<double>>{}, yn.cols, (int)DERowIdx.size()};

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

  matrix<double, colsT, rowsT> xn1New = {std::vector<std::vector<double>>(f.rows, std::vector<double>(f.cols, 0.0)), f.cols, f.rows};
  i = 0;
  for (auto row : DERowIdx) {
    xn1New.data[row][0] = xn1.data[i][0];
    i++;
  }
  
  // AE Equations
  matrix<double, colsT, rowsT> An1 = A;
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

  matrix<double, colsT, rowsT> AEsolsNew = {std::vector<std::vector<double>>(
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
