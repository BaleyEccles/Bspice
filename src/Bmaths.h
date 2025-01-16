#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <utility>
#include <algorithm>
#include <cmath>
#include <iomanip>

struct symbol {
  std::string name;
  symbol(const std::string& n);
  symbol() : name("NULL") {}
};

template<typename T>
struct matrix {
  std::vector<std::vector<T>> data;
  int cols, rows;

  matrix<T> scale(double scale) {
    std::vector<std::vector<T>> outputData(rows, std::vector<T>(cols, 0));
    for (int i = 0; i < rows; ++i) {
      for (int j = 0; j < cols; ++j) {
        outputData[j][i] = scale*data[j][i];
      }
    }
    matrix<T> output = {
      outputData,
      cols, rows
    };
    return output;
  }

  matrix<T> getColumn(int col) {
    std::vector<std::vector<T>> output;
    for (int i = 0; i < rows; i++) {
      output.push_back({data[i][col]});
    }
    return matrix<T>{output, 1, rows};
  }
  matrix<T> getRow(int row) {
    std::vector<std::vector<T>> output;
    for (int i = 0; i < cols; i++) {
      output.push_back({data[row][i]});
    }
    return matrix<T>{output, 1, cols};
  }
  
  void print() {
    for (int i = 0; i < rows; i++) {
      for (int j = 0; j < cols; j++) {
        if constexpr (std::is_arithmetic<T>::value) {
          // If T is an arithmetic type (int, float, double, etc.)
          std::cout << std::fixed << std::setprecision(5) << data[i][j] << " ";
        } else if constexpr (std::is_same<T, symbol>::value) {
          // If T is a string
          std::cout << data[i][j].name << " ";
        }
      }
      std::cout << "\n";
    }
    std::cout << "\n";
  }

  matrix<T> transpose() {
    std::vector<std::vector<T>> outputVec(cols, std::vector<T>(rows));
    for (int row = 0; row < rows; row++) {
      for (int col = 0; col < cols; col++) {
        outputVec[col][row] = data[row][col];
      }
    }
    matrix<T> output = {
      outputVec,
      rows, cols
    };
    return output;
  };


  matrix<T> pseudoInvert() {
    std::vector<std::vector<T>> outputData(rows, std::vector<T>(cols, 0));

    for (int row = 0; row < rows; ++row) {
      for (int col = 0; col < cols; ++col) {
        if (data[row][col] != 0) {
          outputData[row][col] = 1 / data[row][col];
        } else {
          outputData[row][col] = 0;
        }
      }
    }
    matrix<T> output = {
      outputData,
      cols, rows
    };
    return output;
  };
  matrix<T> invert() {
    int n = rows;
    std::vector<std::vector<double>> inverse = std::vector<std::vector<double>>(n, std::vector<double>(n, 0));
    std::vector<std::vector<double>> augmented(n, std::vector<double>(2 * n, 0));

    // Create the augmented matrix [input | I]
    for (int i = 0; i < n; i++) {
      for (int j = 0; j < n; j++) {
        augmented[i][j] = data[i][j];
      }
      augmented[i][i + n] = 1; // Identity matrix
    }

    // Apply Gaussian elimination
    for (int i = 0; i < n; i++) {
      // Search for maximum in this column
      double maxEl = abs(augmented[i][i]);
      int maxRow = i;
      for (int k = i + 1; k < n; k++) {
        if (abs(augmented[k][i]) > maxEl) {
          maxEl = abs(augmented[k][i]);
          maxRow = k;
        }
      }

      // Swap maximum row with current row
      for (int k = i; k < 2 * n; k++) {
        std::swap(augmented[maxRow][k], augmented[i][k]);
      }

      // Make the diagonal contain all 1s
      double diag = augmented[i][i];
      if (diag == 0) {
        print();
        std::cerr << "ERROR: Matrix is singular" << std::endl;
      }
      for (int k = 0; k < 2 * n; k++) {
        augmented[i][k] /= diag;
      }

      // Make the other rows contain 0s in this column
      for (int k = 0; k < n; k++) {
        if (k != i) {
          double factor = augmented[k][i];
          for (int j = 0; j < 2 * n; j++) {
            augmented[k][j] -= factor * augmented[i][j];
          }
        }
      }
    }

    // Extract the inverse matrix from the augmented matrix
    for (int i = 0; i < n; i++) {
      for (int j = 0; j < n; j++) {
        inverse[i][j] = augmented[i][j + n];
      }
    }
    matrix<T> inv = {inverse, cols, rows};
    return inv;
  }
};

template<typename T>
matrix<T> multiply(matrix<T> A, matrix<T> B) {
  if (A.cols != B.rows) {
    std::cerr << "ERROR: cols of A must be equal to rows of B" << std::endl;
    return matrix<T>();
  }

  std::vector<std::vector<T>> outputData(A.rows, std::vector<T>(B.cols, 0));

  for (int row = 0; row < A.rows; ++row) {
    for (int col = 0; col < B.cols; ++col) {
      for (int k = 0; k < A.cols; ++k) {
        outputData[row][col] += A.data[row][k] * B.data[k][col];
      }
    }
  }
  matrix<T> output = {
    outputData,
    B.cols, A.rows
  };

  return output;
}

template<typename T>
matrix<T> subtract(matrix<T> A, matrix<T> B) {
  if (A.cols != B.cols || A.rows != B.rows) { std::cerr << "ERROR: Mismatched matrix sizes" << std::endl; }
  std::vector<std::vector<T>> outputData(A.rows, std::vector<T>(A.cols, 0));
  for (int row = 0; row < A.rows; ++row) {
    for (int col = 0; col < A.cols; ++col) {
      outputData[row][col] = A.data[row][col] - B.data[row][col];
    }
  }
  matrix<T> output = {
    outputData,
    A.cols, A.rows
  };
  return output;
}

template<typename T>
matrix<T> add(matrix<T> A, matrix<T> B) {
  if (A.cols != B.cols || A.rows != B.rows) { std::cerr << "ERROR: Mismatched matrix sizes" << std::endl; }
  std::vector<std::vector<T>> outputData(A.rows, std::vector<T>(A.cols, 0));
  for (int row = 0; row < A.rows; ++row) {
    for (int col = 0; col < A.cols; ++col) {
      outputData[row][col] = A.data[row][col] + B.data[row][col];
    }
  }
  matrix<T> output = {
    outputData,
    A.cols, A.rows
  };
  return output;
}


struct equation {
  std::vector<std::pair<double, symbol>> equation;

  double solve(symbol sym, const std::vector<std::pair<symbol, double>>& values);
};
